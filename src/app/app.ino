/**
 * @file cushion_fsr_monitor.ino
 * @brief ESP32 cushion posture monitor using 4 FSR sensors, 5 LEDs, and FreeRTOS tasks.
 *
 * @details
 * This program reads four FSR (Force Sensitive Resistor) sensors placed on a seat cushion:
 * - Right Bottom  -> GPIO 33
 * - Right Top     -> GPIO 32
 * - Left Top      -> GPIO 35
 * - Left Bottom   -> GPIO 34
 *
 * Based on the summed left and right pressure values, the system classifies posture into:
 * - NO_SIT
 * - LEAN_LEFT
 * - PROPER
 * - LEAN_RIGHT
 *
 * LED behavior:
 * - GPIO 2  -> onboard heartbeat LED (system alive indicator)
 * - GPIO 22 -> reserved for future use
 * - GPIO 21 -> leaning right
 * - GPIO 19 -> proper posture
 * - GPIO 18 -> leaning left
 *
 * FreeRTOS tasks are used for:
 * - heartbeat blinking
 * - sensor reading and posture update
 * - serial logging
 *
 * @note
 * This version uses raw ADC readings. You will likely want to add:
 * - moving average smoothing
 * - per-sensor calibration
 * - front/back posture logic
 * later.
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

/**
 * @brief Logging macro for simple text messages.
 * @param tag Module tag, e.g. "SYS", "FSR", "POSTURE".
 * @param msg Message string.
 */
#define LOG(tag, msg)            \
  do {                           \
    Serial.print("[");           \
    Serial.print(tag);           \
    Serial.print("] ");          \
    Serial.println(msg);         \
  } while (0)

/**
 * @brief Logging macro for formatted messages.
 * @param tag Module tag, e.g. "SYS", "FSR", "POSTURE".
 * @param fmt printf-style format string.
 */
#define LOGF(tag, fmt, ...)      \
  do {                           \
    Serial.print("[");           \
    Serial.print(tag);           \
    Serial.print("] ");          \
    Serial.printf(fmt, ##__VA_ARGS__); \
    Serial.println();            \
  } while (0)

/**
 * @brief Represents the current interpreted seating posture.
 */
enum class PostureState {
  NO_SIT = 0,
  LEAN_LEFT,
  PROPER,
  LEAN_RIGHT
};

/**
 * @brief Stores all relevant FSR sensor readings and computed sums.
 */
struct SensorData {
  int rightBottom = 0;
  int rightTop    = 0;
  int leftTop     = 0;
  int leftBottom  = 0;

  int leftSum     = 0;
  int rightSum    = 0;
  int totalSum    = 0;
  int leftRightDiff = 0;
};

/**
 * @brief Pin configuration container.
 *
 * @details
 * This struct keeps all hardware pins in one place.
 */
struct PinConfig {
  // FSR pins
  static constexpr uint8_t FSR_RIGHT_BOTTOM = 33;
  static constexpr uint8_t FSR_RIGHT_TOP    = 32;
  static constexpr uint8_t FSR_LEFT_TOP     = 35;
  static constexpr uint8_t FSR_LEFT_BOTTOM  = 34;

  // LED pins
  static constexpr uint8_t LED_STATUS   = 2;
  static constexpr uint8_t LED_RESERVED = 22;
  static constexpr uint8_t LED_RIGHT    = 21;
  static constexpr uint8_t LED_PROPER   = 19;
  static constexpr uint8_t LED_LEFT     = 18;
};

/**
 * @brief Timing configuration for FreeRTOS tasks.
 */
struct TimingConfig {
  static constexpr uint32_t HEARTBEAT_MS = 500;
  static constexpr uint32_t SENSOR_MS    = 100;
  static constexpr uint32_t LOG_MS       = 500;
};

/**
 * @brief Thresholds used for posture classification.
 *
 * @details
 * Tune these values during testing.
 */
struct ThresholdConfig {
  static constexpr int TOTAL_SIT_THRESHOLD = 800;
  static constexpr int LEAN_DIFF_THRESHOLD = 400;
};

/**
 * @brief Main application class for cushion posture monitoring.
 *
 * @details
 * This class owns:
 * - sensor reading
 * - posture classification
 * - LED control
 * - serial logging
 * - FreeRTOS task setup
 *
 * Static task wrapper functions are used because FreeRTOS task entry points must be static/free functions.
 */
class CushionMonitor {
public:
  /**
   * @brief Construct a new CushionMonitor object.
   */
  CushionMonitor()
    : postureState(PostureState::NO_SIT),
      dataMutex(nullptr) {}

  /**
   * @brief Initialize hardware, synchronization objects, and tasks.
   *
   * @details
   * This should be called once from Arduino setup().
   */
  void begin() {
    initializePins();
    initializeMutex();
    printStartupBanner();
    createTasks();
  }

private:
  SensorData sensorData;              ///< Current raw and computed sensor values.
  PostureState postureState;          ///< Current interpreted posture state.
  SemaphoreHandle_t dataMutex;        ///< Mutex protecting shared sensor/posture data.

  /**
   * @brief Initialize GPIO pin modes and default LED states.
   */
  void initializePins() {
    pinMode(PinConfig::LED_STATUS, OUTPUT);
    pinMode(PinConfig::LED_RESERVED, OUTPUT);
    pinMode(PinConfig::LED_RIGHT, OUTPUT);
    pinMode(PinConfig::LED_PROPER, OUTPUT);
    pinMode(PinConfig::LED_LEFT, OUTPUT);

    digitalWrite(PinConfig::LED_STATUS, LOW);
    digitalWrite(PinConfig::LED_RESERVED, LOW);
    digitalWrite(PinConfig::LED_RIGHT, LOW);
    digitalWrite(PinConfig::LED_PROPER, LOW);
    digitalWrite(PinConfig::LED_LEFT, LOW);

    pinMode(PinConfig::FSR_RIGHT_BOTTOM, INPUT);
    pinMode(PinConfig::FSR_RIGHT_TOP, INPUT);
    pinMode(PinConfig::FSR_LEFT_TOP, INPUT);
    pinMode(PinConfig::FSR_LEFT_BOTTOM, INPUT);
  }

  /**
   * @brief Create the mutex used for thread-safe shared data access.
   */
  void initializeMutex() {
    dataMutex = xSemaphoreCreateMutex();

    if (dataMutex == nullptr) {
      LOG("SYS", "Failed to create mutex. Halting system.");
      while (true) {
        digitalWrite(PinConfig::LED_STATUS, !digitalRead(PinConfig::LED_STATUS));
        delay(100);
      }
    }
  }

  /**
   * @brief Print startup information to the serial monitor.
   */
  void printStartupBanner() {
    LOG("SYS", "ESP32 Cushion Monitor Starting...");
    LOG("SYS", "Using OOP + FreeRTOS design");

    LOG("PIN", "FSR RB=D33, RT=D32, LT=D35, LB=D34");
    LOG("PIN", "LED STATUS=D2, RESERVED=D22, RIGHT=D21, PROPER=D19, LEFT=D18");

    LOGF("CFG", "TOTAL_SIT_THRESHOLD = %d", ThresholdConfig::TOTAL_SIT_THRESHOLD);
    LOGF("CFG", "LEAN_DIFF_THRESHOLD = %d", ThresholdConfig::LEAN_DIFF_THRESHOLD);
  }

  /**
   * @brief Create all FreeRTOS tasks used by the application.
   */
  void createTasks() {
    xTaskCreatePinnedToCore(
      heartbeatTaskEntry,
      "HeartbeatTask",
      2048,
      this,
      1,
      nullptr,
      0
    );

    xTaskCreatePinnedToCore(
      sensorTaskEntry,
      "SensorTask",
      4096,
      this,
      2,
      nullptr,
      1
    );

    xTaskCreatePinnedToCore(
      loggingTaskEntry,
      "LoggingTask",
      4096,
      this,
      1,
      nullptr,
      1
    );

    LOG("SYS", "All tasks created successfully");
  }

  /**
   * @brief Read all four FSR sensors and update internal raw sensor values.
   *
   * @note
   * This reads raw ADC values only. No smoothing is applied here yet.
   */
  void readSensors() {
    SensorData localData;

    localData.rightBottom = analogRead(PinConfig::FSR_RIGHT_BOTTOM);
    localData.rightTop    = analogRead(PinConfig::FSR_RIGHT_TOP);
    localData.leftTop     = analogRead(PinConfig::FSR_LEFT_TOP);
    localData.leftBottom  = analogRead(PinConfig::FSR_LEFT_BOTTOM);

    localData.leftSum = localData.leftTop + localData.leftBottom;
    localData.rightSum = localData.rightTop + localData.rightBottom;
    localData.totalSum = localData.leftSum + localData.rightSum;
    localData.leftRightDiff = localData.leftSum - localData.rightSum;

    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      sensorData = localData;
      xSemaphoreGive(dataMutex);
    }
  }

  /**
   * @brief Compute posture state from current sensor sums.
   *
   * @details
   * Logic:
   * - If total pressure is below threshold -> NO_SIT
   * - If left side is much larger than right -> LEAN_LEFT
   * - If right side is much larger than left -> LEAN_RIGHT
   * - Otherwise -> PROPER
   */
  void classifyPosture() {
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      if (sensorData.totalSum < ThresholdConfig::TOTAL_SIT_THRESHOLD) {
        postureState = PostureState::NO_SIT;
      }
      else if (sensorData.leftRightDiff > ThresholdConfig::LEAN_DIFF_THRESHOLD) {
        postureState = PostureState::LEAN_LEFT;
      }
      else if (sensorData.leftRightDiff < -ThresholdConfig::LEAN_DIFF_THRESHOLD) {
        postureState = PostureState::LEAN_RIGHT;
      }
      else {
        postureState = PostureState::PROPER;
      }

      xSemaphoreGive(dataMutex);
    }
  }

  /**
 * @brief Update LEDs based on posture state.
 *
 * Mapping:
 * - NO_SIT      -> LED_RESERVED (GPIO 22)
 * - LEAN_LEFT   -> LED_LEFT (GPIO 18)
 * - PROPER      -> LED_PROPER (GPIO 19)
 * - LEAN_RIGHT  -> LED_RIGHT (GPIO 21)
 */
void updatePostureLEDs() {
  PostureState localPosture;

  if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    localPosture = postureState;
    xSemaphoreGive(dataMutex);
  } else {
    return;
  }

  // Turn everything OFF first
  digitalWrite(PinConfig::LED_LEFT, LOW);
  digitalWrite(PinConfig::LED_PROPER, LOW);
  digitalWrite(PinConfig::LED_RIGHT, LOW);
  digitalWrite(PinConfig::LED_RESERVED, LOW);

  // Turn ON based on posture
  switch (localPosture) {
    case PostureState::LEAN_LEFT:
      digitalWrite(PinConfig::LED_LEFT, HIGH);
      break;

    case PostureState::PROPER:
      digitalWrite(PinConfig::LED_PROPER, HIGH);
      break;

    case PostureState::LEAN_RIGHT:
      digitalWrite(PinConfig::LED_RIGHT, HIGH);
      break;

    case PostureState::NO_SIT:
      digitalWrite(PinConfig::LED_RESERVED, HIGH);
      break;

    default:
      break;
  }
}

  /**
   * @brief Toggle the onboard heartbeat LED.
   *
   * @details
   * This LED is independent of posture logic and is only used to show that the ESP32 is alive.
   */
  void heartbeat() {
    digitalWrite(PinConfig::LED_STATUS, !digitalRead(PinConfig::LED_STATUS));
  }

  /**
   * @brief Print current sensor values and posture state to the serial monitor.
   */
  void logStatus() {
    SensorData localData;
    PostureState localPosture;

    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      localData = sensorData;
      localPosture = postureState;
      xSemaphoreGive(dataMutex);
    } else {
      LOG("SYS", "Failed to acquire mutex for logging");
      return;
    }

    LOGF("FSR", "RB:%4d | RT:%4d | LT:%4d | LB:%4d",
         localData.rightBottom,
         localData.rightTop,
         localData.leftTop,
         localData.leftBottom);

    LOGF("SUM", "LEFT:%4d | RIGHT:%4d | TOTAL:%4d | DIFF(L-R):%4d",
         localData.leftSum,
         localData.rightSum,
         localData.totalSum,
         localData.leftRightDiff);

    LOGF("POSTURE", "State: %s", postureToString(localPosture));

    Serial.println("--------------------------------------------------");
  }

  /**
   * @brief Convert posture enum into human-readable text.
   * @param state Posture state enum.
   * @return const char* Human-readable label.
   */
  const char* postureToString(PostureState state) const {
    switch (state) {
      case PostureState::LEAN_LEFT:  return "LEAN_LEFT";
      case PostureState::PROPER:     return "PROPER";
      case PostureState::LEAN_RIGHT: return "LEAN_RIGHT";
      case PostureState::NO_SIT:
      default:                       return "NO_SIT";
    }
  }

  /**
   * @brief Main sensor task loop.
   *
   * @details
   * Repeatedly:
   * - reads sensors
   * - classifies posture
   * - updates posture LEDs
   */
  void runSensorTask() {
    while (true) {
      readSensors();
      classifyPosture();
      updatePostureLEDs();

      vTaskDelay(pdMS_TO_TICKS(TimingConfig::SENSOR_MS));
    }
  }

  /**
   * @brief Main heartbeat task loop.
   *
   * @details
   * Repeatedly toggles onboard LED to indicate system is alive.
   */
  void runHeartbeatTask() {
    while (true) {
      heartbeat();
      vTaskDelay(pdMS_TO_TICKS(TimingConfig::HEARTBEAT_MS));
    }
  }

  /**
   * @brief Main logging task loop.
   *
   * @details
   * Repeatedly prints sensor and posture information to serial.
   */
  void runLoggingTask() {
    while (true) {
      logStatus();
      vTaskDelay(pdMS_TO_TICKS(TimingConfig::LOG_MS));
    }
  }

  /**
   * @brief Static FreeRTOS wrapper for sensor task.
   * @param parameter Pointer to CushionMonitor instance.
   */
  static void sensorTaskEntry(void* parameter) {
    CushionMonitor* self = static_cast<CushionMonitor*>(parameter);
    self->runSensorTask();
  }

  /**
   * @brief Static FreeRTOS wrapper for heartbeat task.
   * @param parameter Pointer to CushionMonitor instance.
   */
  static void heartbeatTaskEntry(void* parameter) {
    CushionMonitor* self = static_cast<CushionMonitor*>(parameter);
    self->runHeartbeatTask();
  }

  /**
   * @brief Static FreeRTOS wrapper for logging task.
   * @param parameter Pointer to CushionMonitor instance.
   */
  static void loggingTaskEntry(void* parameter) {
    CushionMonitor* self = static_cast<CushionMonitor*>(parameter);
    self->runLoggingTask();
  }
};

// ==========================================================
// GLOBAL APPLICATION OBJECT
// ==========================================================

/**
 * @brief Global application instance.
 *
 * @details
 * This object owns the entire posture monitoring application.
 */
CushionMonitor app;

// ==========================================================
// ARDUINO ENTRY POINTS
// ==========================================================

/**
 * @brief Arduino setup function.
 *
 * @details
 * Initializes the application and starts all tasks.
 */
void setup() {
  Serial.begin(115200);
  delay(500);
  app.begin();
}

/**
 * @brief Arduino loop function.
 *
 * @details
 * Not used directly because the program is task-based.
 */
void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}