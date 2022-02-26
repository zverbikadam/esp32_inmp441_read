#include "main.h"
#include "credentials.h"
#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFi.h>

WiFiClient espClient;

static uint8_t *wav_file;
uint32_t buffer32[1024];

void init_wav(uint8_t *wav_file_ptr)
{
#define d0(x) (uint8_t)((x)&0xFF)
#define d1(x) (uint8_t)(((x) >> 8) & 0xFF)
#define d2(x) (uint8_t)(((x) >> 16) & 0xFF)
#define d3(x) (uint8_t)(((x) >> 24) & 0xFF)

  wav_file_ptr[0] = 'R';                                                    // Chunk descriptor
  wav_file_ptr[1] = 'I';                                                    //
  wav_file_ptr[2] = 'F';                                                    //
  wav_file_ptr[3] = 'F';                                                    //
  wav_file_ptr[4] = d0(WAV_FILE_SIZE - 8);                                  // Chunk size
  wav_file_ptr[5] = d1(WAV_FILE_SIZE - 8);                                  // This is the size of the
  wav_file_ptr[6] = d2(WAV_FILE_SIZE - 8);                                  // entire file in bytes minus 8 bytes for the
  wav_file_ptr[7] = d3(WAV_FILE_SIZE - 8);                                  // two fields not included in this count
  wav_file_ptr[8] = 'W';                                                    // Format
  wav_file_ptr[9] = 'A';                                                    //
  wav_file_ptr[10] = 'V';                                                   //
  wav_file_ptr[11] = 'E';                                                   //
  wav_file_ptr[12] = 'f';                                                   // Subchunk ID
  wav_file_ptr[13] = 'm';                                                   //
  wav_file_ptr[14] = 't';                                                   //
  wav_file_ptr[15] = ' ';                                                   //
  wav_file_ptr[16] = 0x10;                                                  // Subchunk1Size    16 for PCM
  wav_file_ptr[17] = 0x00;                                                  //
  wav_file_ptr[18] = 0x00;                                                  //
  wav_file_ptr[19] = 0x00;                                                  //
  wav_file_ptr[20] = 0x01;                                                  // AudioFormat      PCM = 1
  wav_file_ptr[21] = 0x00;                                                  //
  wav_file_ptr[22] = d0(NUM_CHANNELS);                                      // NumChannels      Stereo = 2, Mono = 1
  wav_file_ptr[23] = d1(NUM_CHANNELS);                                      //
  wav_file_ptr[24] = d0(SAMPLE_RATE);                                       // Sample rate
  wav_file_ptr[25] = d1(SAMPLE_RATE);                                       //
  wav_file_ptr[26] = d2(SAMPLE_RATE);                                       //
  wav_file_ptr[27] = d3(SAMPLE_RATE);                                       //
  wav_file_ptr[28] = d0(SAMPLE_RATE * WAV_BYTES_PER_SAMPLE * NUM_CHANNELS); // ByteRate = SampleRate * NumChannels * BitsPerSample/8
  wav_file_ptr[29] = d1(SAMPLE_RATE * WAV_BYTES_PER_SAMPLE * NUM_CHANNELS); //
  wav_file_ptr[30] = d2(SAMPLE_RATE * WAV_BYTES_PER_SAMPLE * NUM_CHANNELS); //
  wav_file_ptr[31] = d3(SAMPLE_RATE * WAV_BYTES_PER_SAMPLE * NUM_CHANNELS); //
  wav_file_ptr[32] = d0(WAV_BYTES_PER_SAMPLE * NUM_CHANNELS);               // BlockAlign = NumChannels * BitsPerSample/8
  wav_file_ptr[33] = d1(WAV_BYTES_PER_SAMPLE * NUM_CHANNELS);               //
  wav_file_ptr[34] = d0(BITS_PER_SAMPLE);                                   // BitsPerSample  8 bits = 8, 16 bits = 16, etc.
  wav_file_ptr[35] = d1(BITS_PER_SAMPLE);                                   //
  wav_file_ptr[36] = 'd';                                                   // Subchunk2ID 'data'
  wav_file_ptr[37] = 'a';                                                   //
  wav_file_ptr[38] = 't';                                                   //
  wav_file_ptr[39] = 'a';                                                   //
  wav_file_ptr[40] = d0(WAV_FILE_DATA_SIZE);                                // Subchunk2Size
  wav_file_ptr[41] = d1(WAV_FILE_DATA_SIZE);                                // NumSamples * NumChannels * BitsPerSample/8
  wav_file_ptr[42] = d2(WAV_FILE_DATA_SIZE);                                // This is the number of bytes in the data.
  wav_file_ptr[43] = d3(WAV_FILE_DATA_SIZE);                                //
}

void init_i2s()
{
  // Config struct
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = 0,
      .dma_buf_count = DMA_BUF_COUNT,
      .dma_buf_len = DMA_BUF_LEN,
      .use_apll = false,
  };

  // Config GPIO
  const i2s_pin_config_t pin_config = {
      .bck_io_num = PIN_I2S_SCK,
      .ws_io_num = PIN_I2S_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = PIN_I2S_SD};

  // Init driver
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

  // Init GPIO
  i2s_set_pin(I2S_PORT, &pin_config);
}

void read_data(uint8_t *wav_file_ptr)
{
  uint32_t bytes_read = 0;
  uint32_t samples_written = 0;
  uint16_t samples_read;
  Serial.println("Recording starting...");
  delay(1000);
  while (samples_written < WAV_FILE_SAMPLES)
  {
    i2s_read(I2S_PORT, (void *)buffer32, sizeof(buffer32), &bytes_read, portMAX_DELAY);

    samples_read = bytes_read >> 2;
    Serial.print("Samples read: ");
    Serial.println(samples_read);

    for (uint32_t i = 0; i < samples_read; i++)
    {
      uint8_t lsb = (buffer32[i] >> 16) & 0xFF;
      uint8_t msb = (buffer32[i] >> 24) & 0xFF;
      uint16_t sample = ((((uint16_t)msb) << 8) | ((uint16_t)lsb)) << 4;

      // Serial.printf("%ld\n", buffer32[i]);
      // Serial.printf("%ld\n", sample);

      if (samples_written < WAV_FILE_SAMPLES)
      {
        wav_file_ptr[WAV_FILE_HEADER_SIZE + (samples_written * 2)] = (uint8_t)sample & 0xFF;
        wav_file_ptr[WAV_FILE_HEADER_SIZE + (samples_written * 2 + 1)] = (uint8_t)(sample >> 8);
        samples_written++;
      }
      else
      {
        break;
      }
    }
  }
}

void upload_data(uint8_t *wav_file_ptr)
{
  if (WiFi.isConnected()) // WiFi Router - STA mode
  // if(WiFi.softAPgetStationNum()>0)                            // WiFi Access point - Soft-AP
  {
    while (espClient.connect(SERVER_IP, SERVER_PORT) != 1)
    {
      Serial.println("Connection to server failed, retrying...");
      delay(500);
    }
    // Upload data to server
    Serial.println("Uploading data!");
    espClient.write(wav_file_ptr, WAV_FILE_SIZE);

    espClient.stop();
  }
  else
  {
    Serial.println("Wifi not connected");
  }
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);

  setup_wifi();
  Serial.print("Start!");

  wav_file = (uint8_t *)malloc(WAV_FILE_SIZE * sizeof(uint8_t));
  if (wav_file == NULL)
  {
    Serial.println("Fatal error, not enough memory for WAV file!");
    while (1)
      ;
  }

  init_wav(wav_file);
  init_i2s();

  delay(500);
}

void loop()
{
  read_data(wav_file);
  upload_data(wav_file);
}