#ifndef main_h
#define main_h

// Set pins
#define PIN_I2S_WS GPIO_NUM_22  // Word Select
#define PIN_I2S_SD GPIO_NUM_21  // Serial Data
#define PIN_I2S_SCK GPIO_NUM_26 // Serial Clock

// Config I2S
#define I2S_PORT I2S_NUM_0
#define I2S_READ_LEN (16 * 1024)

// Config DMA
#define DMA_BUF_COUNT 16 
#define DMA_BUF_LEN 64

// Config audio
#define BITS_PER_SAMPLE 16
#define SAMPLE_RATE 16000
#define NUM_CHANNELS 1

// Config WAV file
#define WAV_FILE_SECONDS 1 

// Const setup
#define WAV_FILE_SAMPLES (WAV_FILE_SECONDS * SAMPLE_RATE)
#define WAV_FILE_HEADER_SIZE 44
#define WAV_BYTES_PER_SAMPLE (BITS_PER_SAMPLE / 8)
#define WAV_FILE_DATA_SIZE (WAV_FILE_SAMPLES * WAV_BYTES_PER_SAMPLE)
#define WAV_FILE_SIZE (WAV_FILE_HEADER_SIZE + WAV_FILE_DATA_SIZE)
#define FLASH_RECORD_SIZE (NUM_CHANNELS * SAMPLE_RATE * BITS_PER_SAMPLE / 8 * WAV_FILE_SECONDS)
#endif
