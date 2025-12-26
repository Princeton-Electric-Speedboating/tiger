#include <Arduino.h>
#include <CAN.h>

// Pin definition
#define IGNITION_PIN 2
#define OVERRIDE_PIN 4
#define PRECHARGE_RELAY 15
#define BATTERY_CONTACTOR 16

TaskHandle_t ignitionTaskHandle = NULL;

// Interrupt Service Routine for ignition signal
void IRAM_ATTR ignitionISR()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(ignitionTaskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken)
        portYIELD_FROM_ISR();
}

// Enable safe startup and shutdown of the vessel by controlling precharge timing.
// Also enables startup/shutdown of BMS Control Unit and Cooling.
// These can be overridden physically using the Override Switch on the dashboard.
void ignitionTask(void *pvParameters)
{
    for (;;)
    {
        // Wait for interrupt notification
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (digitalRead(OVERRIDE_PIN) == HIGH) continue;

        if (digitalRead(IGNITION_PIN) == HIGH) {
            // Startup sequence
            digitalWrite(PRECHARGE_RELAY, HIGH);
            vTaskDelay(pdMS_TO_TICKS(10000));
            digitalWrite(BATTERY_CONTACTOR, HIGH);
            digitalWrite(PRECHARGE_RELAY, LOW);
        } else {
            // Shutdown sequence
            digitalWrite(PRECHARGE_RELAY, HIGH);
            digitalWrite(BATTERY_CONTACTOR, LOW);
            vTaskDelay(pdMS_TO_TICKS(10000));
            digitalWrite(PRECHARGE_RELAY, LOW);
        }

        // Debug print to indicate task is running
        Serial.printf("Ignition event on core %d\n", xPortGetCoreID());
    }
}

// Request and read CAN message from the Powertrain CAN Bus,
// getting data such as cell state of charge and motor temperature.
void powertrainTask(void *pvParameters)
{
    for (;;)
    {
        // TODO: Implement CAN message requests and reading here


        // Debug print to indicate task is running
        Serial.printf("Powertrain running on core %d\n", xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Send data to the dashboard to display.
void dashboardTask(void *pvParameters)
{
    for (;;)
    {
        // TODO: Implement dashboard data sending here

      
        // Debug print to indicate task is running
        Serial.printf("Dashboard running on core %d\n", xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
// Store all task information into onboard MicroSD card to be able to review for testing.
void storageTask(void *pvParameters)
{
    for (;;)
    {
        // TODO: Implement data storage to MicroSD card here

        // Debug print to indicate task is running
        Serial.printf("Storage running on core %d\n", xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(IGNITION_PIN, INPUT_PULLDOWN);

    // Initialize tasks
    xTaskCreatePinnedToCore(powertrainTask, "Powertrain", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(ignitionTask,   "Ignition",   4096, NULL, 5, &ignitionTaskHandle, 1);
    xTaskCreatePinnedToCore(dashboardTask,  "Dashboard",  4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(storageTask,    "Storage",    4096, NULL, 1, NULL, 0);

    // Attach interrupts
    attachInterrupt(digitalPinToInterrupt(IGNITION_PIN), ignitionISR, RISING);

    Serial.println("System started.");
}

void loop()
{
}
