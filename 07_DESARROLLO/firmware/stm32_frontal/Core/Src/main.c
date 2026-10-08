/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "libjpeg.h"
#include "app_touchgfx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SD_LOG_INTERVAL_MS  200
#define SENSOR_READ_INTERVAL_MS 20
#define HALL_CALC_INTERVAL_MS 100


#define CAN_ID_ESP32_DATA   0x123

#define CAN_ID_KART_CONTROL       0x100
#define ADC_ACCEL_CHANNEL      ADC_CHANNEL_0  // PA0
#define ADC_STEER_CHANNEL      ADC_CHANNEL_8  // PF10
#define BRAKE_PIN              GPIO_PIN_6     // PG6
#define BRAKE_PORT             GPIOG
#define MODE_D_PIN             GPIO_PIN_7     // PC7 - Drive (D)
#define MODE_D_PORT            GPIOC
#define MODE_R_PIN             GPIO_PIN_6     // PC6 - Reverse (R)
#define MODE_R_PORT            GPIOC
#define CAN_ID_MODO_CONTROLO      0x104
#define MODO_DIFERENCIAL         0x00
#define MODO_DIFERENCIAL_ESC     0x01
#define MODO_AUTOBLOCANTE        0x02
#define LCD_FB_START_ADDRESS       ((uint32_t)0xC0000000)
#define ACCEL_NOISE_THRESHOLD  100

#define REFRESH_COUNT        1835

#define ACCEL_ADC_MIN  124    // 0.1V -> 124
#define ACCEL_ADC_MAX  1861   // 1.5V -> 1861

#define SDRAM_TIMEOUT                            ((uint32_t)0xFFFF)
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

#define FLASH_READ_JEDEC_ID				0x9F
#define FLASH_FAST_READ_QUAD_IO		 	0xEB
#define FLASH_REVC03_MANUFACTURER_ID	0xEF
#define FLASH_N25Q128A_DUMMY_CYCLES		0x0A
#define FLASH_W25Q128J_DUMMY_CYCLES		0x06


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc3;

CAN_HandleTypeDef hcan1;

CRC_HandleTypeDef hcrc;

DMA2D_HandleTypeDef hdma2d;

I2C_HandleTypeDef hi2c3;

LTDC_HandleTypeDef hltdc;

QSPI_HandleTypeDef hqspi;

SD_HandleTypeDef hsd1;
DMA_HandleTypeDef hdma_sdmmc1_rx;
DMA_HandleTypeDef hdma_sdmmc1_tx;

SDRAM_HandleTypeDef hsdram1;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 4096,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TouchGFXTask */
osThreadId_t TouchGFXTaskHandle;
const osThreadAttr_t TouchGFXTask_attributes = {
  .name = "TouchGFXTask",
  .stack_size = 4096 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for videoTask */
osThreadId_t videoTaskHandle;
const osThreadAttr_t videoTask_attributes = {
  .name = "videoTask",
  .stack_size = 1000 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for SDTask */
osThreadId_t SDTaskHandle;
const osThreadAttr_t SDTask_attributes = {
  .name = "SDTask",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* USER CODE BEGIN PV */
extern void* screen1View;
extern void* screen2View;

static FMC_SDRAM_CommandTypeDef Command;

//Cartao SD
volatile uint8_t g_sd_record_enable = 0;  // 1 = a gravar, 0 = parado

// ===== VARIÁVEIS GLOBAIS CAN → TouchGFX =====
volatile uint8_t g_can_data_buffer[8] = {0};
volatile uint8_t g_can_new_data_flag = 0;
volatile uint32_t g_can_received_id = 0;

volatile int16_t g_velocidade_kph = 0;
volatile int16_t g_vel_roda_esq = 0;
volatile int16_t g_vel_roda_dir = 0;
volatile uint8_t g_modo_kart = 2;
volatile int16_t g_yaw_rate = 0;

volatile uint32_t g_pulsos_di_esq = 0;
volatile uint32_t g_pulsos_di_dir = 0;
volatile int16_t g_vel_roda_di_esq = 0;
volatile int16_t g_vel_roda_di_dir = 0;


volatile uint8_t g_brake_pressed = 0;
volatile uint8_t g_botao_start = 0;   // 1 = premido, 0 = solto

// ===== Dados do BMS (Pylontech) =====
volatile uint16_t g_bms_soc = 0;
volatile int16_t g_bms_temp = 0;

volatile uint16_t g_steering_raw = 0;   // valor bruto da direção (ADC 0-4095)

// ===== VARIÁVEIS DO SD CARD =====
FATFS fs;
FIL fil;
FRESULT fres;
UINT bytes_written;
volatile uint8_t sd_ready = 0;
uint32_t last_sd_log_time = 0;

// ===== Estrutura de Dados dos Sensores =====
typedef struct {
    uint16_t accel_raw;
    uint16_t steer_raw;
    uint8_t  brake;
    uint8_t  mode_d;
    uint8_t  mode_r;
} SensorData_t;



// ===== Variáveis de controlo =====
volatile uint8_t g_modo_controlo = 0;
uint8_t marcha_atual = 2; // 0=D, 1=R, 2=N
uint32_t tick_anterior = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CRC_Init(void);
static void MX_DMA2D_Init(void);
static void MX_FMC_Init(void);
static void MX_I2C3_Init(void);
static void MX_LTDC_Init(void);
static void MX_QUADSPI_Init(void);
static void MX_CAN1_Init(void);
static void MX_ADC3_Init(void);
static void MX_SDMMC1_SD_Init(void);
void StartDefaultTask(void *argument);
void StartSDTask(void *argument);
extern void TouchGFX_Task(void *argument);
extern void videoTaskFunc(void *argument);

/* USER CODE BEGIN PFP */
void SD_Init(void);
void SD_Log(SensorData_t *data);
void Close_SD(void);
void Read_Sensors(SensorData_t *data);
void Send_Data_CAN(SensorData_t *data);

void GetManufacturerId (uint8_t *manufacturer_id);
void EnableMemoryMappedMode(uint8_t manufacturer_id);
void Atualizar_Display_Velocidade(void);
void atualizarTela1(void);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance == CAN1)
    {
        CAN_RxHeaderTypeDef rxHeader;
        uint8_t data[8];

        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, data) == HAL_OK)
        {
            // ===== RECEBER DADOS DO ESP32 (ID 0x123) =====
            if (rxHeader.StdId == 0x123 && rxHeader.DLC >= 8)
            {
                g_velocidade_kph = (int16_t)((data[0] << 8) | data[1]);
                g_vel_roda_esq   = (int16_t)((data[2] << 8) | data[3]);
                g_vel_roda_dir   = (int16_t)((data[4] << 8) | data[5]);

                // ===== EXTRAI O YAW DOS BYTES 6 E 7 =====
                g_yaw_rate = (int16_t)((data[6] << 8) | data[7]);

                g_can_new_data_flag = 1;
            }

            // ===== DADOS DO BMS (Pylontech) =====
            // SOC (ID 0x355, bytes 4-5, little-endian, 0.01%)
            if (rxHeader.StdId == 0x355)
            {
                uint16_t soc_raw = ((uint16_t)data[5] << 8) | data[4];
                g_bms_soc = soc_raw / 100;
            }
            // Temperatura (ID 0x356, bytes 4-5, little-endian, 0.1 °C)
            else if (rxHeader.StdId == 0x356)
            {
                int16_t temp_raw = (int16_t)(((uint16_t)data[5] << 8) | data[4]);
                g_bms_temp = temp_raw / 10;
            }
        }
    }
}

void GetManufacturerId(uint8_t *manufacturer_id)
{
    QSPI_CommandTypeDef s_command_id;
    s_command_id.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command_id.Instruction       = FLASH_READ_JEDEC_ID;
    s_command_id.AddressMode       = QSPI_ADDRESS_NONE;
    s_command_id.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command_id.DataMode          = QSPI_DATA_1_LINE;
    s_command_id.DummyCycles       = 0;
    s_command_id.NbData            = 1;
    s_command_id.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command_id.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command_id.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    if (HAL_QSPI_Command(&hqspi, &s_command_id, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_QSPI_Receive(&hqspi, manufacturer_id, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        Error_Handler();
    }
}

void EnableMemoryMappedMode(uint8_t manufacturer_id)
{
    QSPI_CommandTypeDef      s_command;
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = FLASH_FAST_READ_QUAD_IO;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_4_LINES;
    s_command.DummyCycles       = manufacturer_id == FLASH_REVC03_MANUFACTURER_ID ? FLASH_W25Q128J_DUMMY_CYCLES : FLASH_N25Q128A_DUMMY_CYCLES;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod     = 0;
    if (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK)
    {
        Error_Handler();
    }
}

void CAN_Init_Hardware(void)
{
    HAL_CAN_Start(&hcan1);

    CAN_FilterTypeDef filter;
    filter.FilterBank = 0;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterIdHigh = 0x0000;
    filter.FilterIdLow = 0x0000;
    filter.FilterMaskIdHigh = 0x0000;
    filter.FilterMaskIdLow = 0x0000;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterActivation = ENABLE;
    filter.SlaveStartFilterBank = 14;

    HAL_CAN_ConfigFilter(&hcan1, &filter);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void SD_Init(void)
{
    fres = f_mount(&fs, "", 1);
    if (fres != FR_OK) { sd_ready = 0; return; }

    fres = f_open(&fil, "kart_log.csv", FA_OPEN_APPEND | FA_WRITE);
    if (fres != FR_OK) { sd_ready = 0; return; }

    if (f_size(&fil) == 0) {
        char header[] = "Tick,Accel,Steer,Brake,ModeD,ModeR,Vel,DiEsq,DiDir,TrEsq,TrDir,ModoDif,Yaw\r\n";
        f_write(&fil, header, strlen(header), &bytes_written);
        f_sync(&fil);
    }

    f_lseek(&fil, f_size(&fil));
    sd_ready = 1;
}

void SD_Log(SensorData_t *data)
{
    if (!sd_ready) return;

    char line[256];
    int len = sprintf(line, "%lu,%u,%u,%u,%u,%u,%d,%d,%d,%d,%d,%d,%d\r\n",
            HAL_GetTick(),
            data->accel_raw,
            data->steer_raw,
            data->brake,
            data->mode_d,
            data->mode_r,
            g_velocidade_kph,
            g_vel_roda_di_esq,
            g_vel_roda_di_dir,
            g_vel_roda_esq,
            g_vel_roda_dir,
            g_modo_controlo,
            g_yaw_rate);

    if (len > 0) {
        f_write(&fil, line, len, &bytes_written);
    }
}

void Read_Sensors(SensorData_t *data)
{
    // ===== ACELERADOR (ADC3, Canal 0 - PA0) =====
    ADC_ChannelConfTypeDef sConfig;
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
    HAL_ADC_ConfigChannel(&hadc3, &sConfig);

    HAL_ADC_Start(&hadc3);
    if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) {
        uint16_t raw = HAL_ADC_GetValue(&hadc3);
        if (raw < ACCEL_ADC_MIN) raw = ACCEL_ADC_MIN;
        if (raw > ACCEL_ADC_MAX) raw = ACCEL_ADC_MAX;
        data->accel_raw = (uint16_t)(((uint32_t)(raw - ACCEL_ADC_MIN) * 4095) / (ACCEL_ADC_MAX - ACCEL_ADC_MIN));
    }
    HAL_ADC_Stop(&hadc3);

    // ===== DIREÇÃO (ADC3, Canal 8 - PF10) =====
    // Reconfigura o ADC3 para o Canal 8 antes de ler
    sConfig.Channel = ADC_CHANNEL_8;
    HAL_ADC_ConfigChannel(&hadc3, &sConfig);

    HAL_ADC_Start(&hadc3);
    if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) {
        uint16_t raw = HAL_ADC_GetValue(&hadc3);

        // ===== NORMALIZAÇÃO DO VOLANTE (1042 a 3313 → 0 a 4095) =====
        #define STEER_ADC_MIN   1042   // 0,84 V
        #define STEER_ADC_MAX   3313   // 2,67 V

        if (raw < STEER_ADC_MIN) raw = STEER_ADC_MIN;
        if (raw > STEER_ADC_MAX) raw = STEER_ADC_MAX;
        data->steer_raw = (uint16_t)(((uint32_t)(raw - STEER_ADC_MIN) * 4095) / (STEER_ADC_MAX - STEER_ADC_MIN));
    }
    HAL_ADC_Stop(&hadc3);
    g_steering_raw = data->steer_raw;

    // ===== BOTÕES =====
    data->brake = (HAL_GPIO_ReadPin(BRAKE_PORT, BRAKE_PIN) == GPIO_PIN_RESET) ? 1 : 0;
    data->mode_d = HAL_GPIO_ReadPin(MODE_D_GPIO_Port, MODE_D_Pin);
    data->mode_r = HAL_GPIO_ReadPin(MODE_R_GPIO_Port, MODE_R_Pin);

    g_brake_pressed = data->brake;
    g_botao_start = HAL_GPIO_ReadPin(START_STOP_GPIO_Port, START_STOP_Pin);
}
void Send_Data_CAN(SensorData_t *data)
{
    CAN_TxHeaderTypeDef txHeader;
    uint8_t txData[8];
    uint32_t mailbox;

    txHeader.StdId = CAN_ID_KART_CONTROL;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = 8;
    txHeader.TransmitGlobalTime = DISABLE;

    txData[0] = (data->accel_raw >> 8) & 0xFF;
    txData[1] = data->accel_raw & 0xFF;
    txData[2] = (data->steer_raw >> 8) & 0xFF;
    txData[3] = data->steer_raw & 0xFF;
    txData[4] = data->brake;
    txData[5] = data->mode_d;
    txData[6] = data->mode_r;
    txData[7] = (g_modo_controlo & 0x03) | ((g_botao_start & 0x01) << 2);

    HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &mailbox);
}

// Protótipos das funções que vamos usar
void CAN_Init_Hardware(void);
void CAN_Send_Test_Message(void);

void Read_Sensors(SensorData_t* sensor_data);
void Send_Sensor_Data_CAN(SensorData_t* sensor_data);

SensorData_t g_sensor_data = {0};

// Variáveis globais para o CAN (opcional, para debugging)
CAN_TxHeaderTypeDef   TxHeader;
CAN_RxHeaderTypeDef   RxHeader;
uint8_t               TxData[8];
uint8_t               RxData[8];
uint32_t              TxMailbox;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CRC_Init();
  MX_DMA2D_Init();
  MX_FMC_Init();
  MX_I2C3_Init();
  MX_LTDC_Init();
  MX_QUADSPI_Init();
  MX_LIBJPEG_Init();
  MX_CAN1_Init();
  MX_ADC3_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  MX_TouchGFX_Init();
  /* Call PreOsInit function */
  MX_TouchGFX_PreOSInit();
  /* USER CODE BEGIN 2 */


  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of TouchGFXTask */
  TouchGFXTaskHandle = osThreadNew(TouchGFX_Task, NULL, &TouchGFXTask_attributes);

  /* creation of videoTask */
  videoTaskHandle = osThreadNew(videoTaskFunc, NULL, &videoTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  // ===== CRIA A TAREFA DO SD CARD =====
  SDTaskHandle = osThreadNew(StartSDTask, NULL, &SDTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_12TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_5TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = ENABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */
  HAL_Delay(100); //Delay to fix initialization issue on some boards
  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x20404768;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 40;
  hltdc.Init.VerticalSync = 9;
  hltdc.Init.AccumulatedHBP = 53;
  hltdc.Init.AccumulatedVBP = 11;
  hltdc.Init.AccumulatedActiveW = 533;
  hltdc.Init.AccumulatedActiveH = 283;
  hltdc.Init.TotalWidth = 565;
  hltdc.Init.TotalHeigh = 285;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 480;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 272;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = 0xC0000000;
  pLayerCfg.ImageWidth = 480;
  pLayerCfg.ImageHeight = 272;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
static void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 1;
  hqspi.Init.FifoThreshold = 4;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.FlashSize = 24;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_6_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */
  uint8_t manufacturer_id;
  GetManufacturerId (&manufacturer_id);
  EnableMemoryMappedMode(manufacturer_id);
  HAL_NVIC_DisableIRQ(QUADSPI_IRQn);
  /* USER CODE END QUADSPI_Init 2 */

}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDMMC1_Init 2 */

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 2;
  SdramTiming.ExitSelfRefreshDelay = 7;
  SdramTiming.SelfRefreshTime = 4;
  SdramTiming.RowCycleDelay = 7;
  SdramTiming.WriteRecoveryTime = 3;
  SdramTiming.RPDelay = 2;
  SdramTiming.RCDDelay = 2;

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */
  __IO uint32_t tmpmrd = 0;

    /* Step 1: Configure a clock configuration enable command */
    Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
    Command.CommandTarget          =  FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

    /* Step 2: Insert 100 us minimum delay */
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    HAL_Delay(1);

    /* Step 3: Configure a PALL (precharge all) command */
    Command.CommandMode            = FMC_SDRAM_CMD_PALL;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

    /* Step 4: Configure an Auto Refresh command */
    Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 8;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

    /* Step 5: Program the external memory mode register */
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1 | \
             SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL    | \
             SDRAM_MODEREG_CAS_LATENCY_3            | \
             SDRAM_MODEREG_OPERATING_MODE_STANDARD  | \
             SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

    /* Step 6: Set the refresh rate counter */
    /* Set the device refresh rate */
    HAL_SDRAM_ProgramRefreshRate(&hsdram1, REFRESH_COUNT);

    //Deactivate speculative/cache access to first FMC Bank to save FMC bandwidth
    FMC_Bank1->BTCR[0] = 0x000030D2;
  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_BL_CTRL_GPIO_Port, LCD_BL_CTRL_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_STM_GPIO_Port, LED_STM_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_DISP_GPIO_Port, LCD_DISP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : HALL_DI_ESQ_Pin */
  GPIO_InitStruct.Pin = HALL_DI_ESQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(HALL_DI_ESQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_BL_CTRL_Pin */
  GPIO_InitStruct.Pin = LCD_BL_CTRL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BL_CTRL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_STM_Pin LCD_DISP_Pin */
  GPIO_InitStruct.Pin = LED_STM_Pin|LCD_DISP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : START_STOP_Pin */
  GPIO_InitStruct.Pin = START_STOP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(START_STOP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MODE_R_Pin MODE_D_Pin */
  GPIO_InitStruct.Pin = MODE_R_Pin|MODE_D_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : HALL_DI_DIR_Pin */
  GPIO_InitStruct.Pin = HALL_DI_DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(HALL_DI_DIR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BRAKE_Pin */
  GPIO_InitStruct.Pin = BRAKE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(BRAKE_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);  // PG7 é EXTI7, que está no grupo 5-9
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);


  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void Close_SD(void)
{
    if (sd_ready) {
        f_sync(&fil);
        f_close(&fil);
        f_mount(NULL, "", 0);
        f_mount(NULL, "", 1);
        f_mount(NULL, "", 0);
        sd_ready = 0;
    }
}
void StartSDTask(void *argument)
{
    SD_Init(); // Inicializa o SD (tenta montar logo no início)

    uint32_t last_sd_sync = 0;

    for(;;)
    {
        uint32_t tick = osKernelGetTickCount();

        // ===== ESCREVER NO SD APENAS SE O BOTÃO PLAY ESTIVER ATIVO =====
        uint8_t record_now = g_sd_record_enable; // Força a leitura real da variável

        if (record_now == 1 && sd_ready)
        {
            static uint32_t last_sd_log = 0;

            if (tick - last_sd_log >= SD_LOG_INTERVAL_MS)
            {
                last_sd_log = tick;

                // Verifica se o ficheiro está vazio e escreve o cabeçalho
                if (f_size(&fil) == 0) {
                    char header[] = "Tick,Accel,Steer,Brake,ModeD,ModeR,Vel,DiEsq,DiDir,TrEsq,TrDir,ModoDif,Yaw\r\n";
                    f_write(&fil, header, strlen(header), &bytes_written);
                    f_sync(&fil);
                }

                SD_Log(&g_sensor_data);
            }

            // Sincroniza fisicamente com o cartão apenas a cada 1 segundo
            if (tick - last_sd_sync >= 1000) {
                last_sd_sync = tick;
                f_sync(&fil);
            }
        }

        osDelay(SD_LOG_INTERVAL_MS / 2); // Pequeno delay para não sobrecarregar o CPU
    }
}
/* USER CODE END 4 */

void Atualizar_Display_Velocidade(void)
{
    // Chama a função do TouchGFX para atualizar o ecrã
    atualizarTela1();

}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
    CAN_Init_Hardware();

    for(int i=0; i<3; i++) {
        HAL_GPIO_TogglePin(LED_STM_GPIO_Port, LED_STM_Pin);
        osDelay(200);
    }

    uint32_t tick_anterior = 0;
    uint32_t last_display = 0;

    uint8_t last_hall_esq = 1, last_hall_dir = 1;
    uint32_t last_pulse_time_esq = 0, last_pulse_time_dir = 0;
    uint32_t pulse_interval_esq = 0, pulse_interval_dir = 0;
    bool first_pulse_esq = true, first_pulse_dir = true;

    for(;;)
    {
        uint32_t tick = osKernelGetTickCount();

        // ===== 1. LER SENSORES A CADA 5ms =====
        if (tick - tick_anterior >= 5)
        {
            tick_anterior = tick;
            Read_Sensors(&g_sensor_data);

            uint8_t hall_esq = HAL_GPIO_ReadPin(HALL_DI_ESQ_GPIO_Port, HALL_DI_ESQ_Pin);
            uint8_t hall_dir = HAL_GPIO_ReadPin(HALL_DI_DIR_GPIO_Port, HALL_DI_DIR_Pin);

            if (last_hall_esq == 1 && hall_esq == 0) {
                if (!first_pulse_esq) {
                    pulse_interval_esq = tick - last_pulse_time_esq;
                }
                last_pulse_time_esq = tick;
                first_pulse_esq = false;
            }
            last_hall_esq = hall_esq;

            if (last_hall_dir == 1 && hall_dir == 0) {
                if (!first_pulse_dir) {
                    pulse_interval_dir = tick - last_pulse_time_dir;
                }
                last_pulse_time_dir = tick;
                first_pulse_dir = false;
            }
            last_hall_dir = hall_dir;

            if (g_sensor_data.mode_d == 1) {
                if (marcha_atual != 0 && g_sensor_data.brake && g_velocidade_kph == 0) {
                    marcha_atual = 0;
                }
            } else if (g_sensor_data.mode_r == 1) {
                if (marcha_atual != 1 && g_sensor_data.brake && g_velocidade_kph == 0) {
                    marcha_atual = 1;
                }
            } else {
                marcha_atual = 2;
            }

            g_modo_kart = marcha_atual;

            SensorData_t tx_data = g_sensor_data;
            tx_data.mode_d = (marcha_atual == 0);
            tx_data.mode_r = (marcha_atual == 1);
            Send_Data_CAN(&tx_data);

            HAL_GPIO_TogglePin(LED_STM_GPIO_Port, LED_STM_Pin);
        }

        // ===== 2. CALCULAR RPM =====
        if (pulse_interval_esq > 0 && pulse_interval_esq < 5000
            && (tick - last_pulse_time_esq) <= 500) {
            g_vel_roda_di_esq = (60000 / pulse_interval_esq) / 4;
        } else {
            g_vel_roda_di_esq = 0;
        }

        if (pulse_interval_dir > 0 && pulse_interval_dir < 5000
            && (tick - last_pulse_time_dir) <= 500) {
            g_vel_roda_di_dir = (60000 / pulse_interval_dir) / 4;
        } else {
            g_vel_roda_di_dir = 0;
        }

        // ===== 3. ATUALIZAR DISPLAY A CADA 50ms =====
        if ((tick - last_display) >= 50) {
            last_display = tick;
            Atualizar_Display_Velocidade();
        }

        osDelay(5);  // era 1
    }
}


 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x90000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256MB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16MB;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

