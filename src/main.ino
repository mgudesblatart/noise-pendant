#include <Arduino.h>
#include <driver/i2s.h>
#include <U8g2lib.h>

#define BUILTIN_LED 8
#define SAMPLE_BUFFER_SIZE 512
#define SAMPLE_RATE 8000
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_0
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_2
#define I2S_MIC_SERIAL_DATA GPIO_NUM_1

// I2S config
static i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

static i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA};

U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);
int width = 72;
int height = 40;
int xOffset = 0;
int yOffset = 0;

int32_t raw_samples[SAMPLE_BUFFER_SIZE];

void audioTask(void *pvParameters) {
    while (1) {
        size_t bytes_read = 0;
        esp_err_t err = i2s_read(I2S_NUM_0, raw_samples, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
        if (err != ESP_OK) {
            Serial.printf("I2S read failed: %d\n", err);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        int samples_read = bytes_read / sizeof(int32_t);
        // dump the samples out to the serial channel.
        for (int i = 0; i < samples_read; i++) {
            Serial.printf("%ld\n", raw_samples[i]);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // yield to other tasks
    }
}

void displayTask(void *pvParameters) {
    while (1) {
        u8g2.clearBuffer();
        u8g2.drawFrame(xOffset + 0, yOffset + 0, width, height);
        u8g2.setCursor(xOffset + 15, yOffset + 25);
        u8g2.printf("%dx%d", width, height);
        u8g2.sendBuffer();
        vTaskDelay(100 / portTICK_PERIOD_MS); // update every 100ms
    }
}

void setup() {
    delay(1000);
    u8g2.begin();
    u8g2.setContrast(255);
    u8g2.setBusClock(400000);
    u8g2.setFont(u8g2_font_ncenB10_tr);
    Serial.begin(115200);
    pinMode(BUILTIN_LED, OUTPUT);
    delay(1000);
    Serial.println("Serial started. Beginning I2S init...");
    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("I2S driver install failed: %d\n", err);
        while (1) {
            delay(1000);
        }
    }
    i2s_set_pin(I2S_NUM_0, &i2s_mic_pins);
    Serial.println("I2S init complete. Starting FreeRTOS tasks.");
    xTaskCreatePinnedToCore(audioTask, "AudioTask", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(displayTask, "DisplayTask", 4096, NULL, 1, NULL, 1);
}

void loop() {
    vTaskDelay(1000 / portTICK_PERIOD_MS); // main loop does nothing
}
