/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    : main.c
 * @brief   : UART Kontrollü GPIO Çıkış Programı
 *
 * Seçilen Mikrodenetleyici: STM32F103C8T6 ("Blue Pill")
 * Seçim Gerekçesi:
 *   - Düşük maliyet ve yaygın erişilebilirlik
 *   - 72 MHz ARM Cortex-M3, bu görev için yeterli işlem gücü
 *   - 2 adet USART, 37 GPIO pini — UART + GPIO için ideal
 *   - Geniş HAL kütüphanesi ve topluluk desteği
 *   - Elimde bulunan model bu
 *
 * Donanım Bağlantıları:
 *   USART1 TX → PA9   (bilgisayara gönderim, isteğe bağlı)
 *   USART1 RX → PA10  (bilgisayardan mesaj alma)
 *   GPIO Pin 1 → PA0  (çıkış LED/röle vb.)
 *   GPIO Pin 2 → PA1  (çıkış LED/röle vb.)
 *
 * Mesaj Protokolü (2 ASCII karakter):
 *   '0' → LOW   (pin lojik 0'a çek)
 *   '1' → HIGH  (pin lojik 1'e çek)
 *   '2' → TOGGLE (pin mevcut durumu tersine çevir)
 *   İlk karakter  → PA0 (Pin 1)
 *   İkinci karakter → PA1 (Pin 2)
 *
 * Örnek:
 *   "11" → PA0=HIGH,  PA1=HIGH
 *   "10" → PA0=HIGH,  PA1=LOW
 *   "01" → PA0=LOW,   PA1=HIGH
 *   "00" → PA0=LOW,   PA1=LOW
 *   "21" → PA0=TOGGLE, PA1=HIGH
 ******************************************************************************
 */
/* USER CODE END Header */

/* ==================== INCLUDE'LAR ==================== */
#include "main.h"   /* STM32 HAL temel başlık dosyası, hata işleyici ve pin tanımları */

/* ==================== ÖZEL DEĞİŞKENLER ==================== */

/* USART1 için HAL handle: tüm UART işlemlerinde bu yapı kullanılır */
UART_HandleTypeDef huart1;

/* 2 byte'lık UART alma tamponu.
   Her mesaj tam olarak 2 ASCII karakterden oluşur (Pin1_Komutu, Pin2_Komutu) */
uint8_t rxBuffer[2];

/* ==================== FONKSİYON PROTOTİPLERİ ==================== */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
void processUARTMessage(uint8_t *buffer);

/* ==================== MAIN ==================== */
int main(void)
{
    /* HAL (Hardware Abstraction Layer) kütüphanesini başlat.
       Tick zamanlayıcısını (SysTick) ve genel HAL altyapısını hazırlar. */
    HAL_Init();

    /* Sistem saatini yapılandır.
       STM32F103C8T6'yı 72 MHz'de çalıştırmak için PLL ayarları yapılır. */
    SystemClock_Config();

    /* GPIO pinlerini (PA0, PA1) çıkış olarak başlat */
    MX_GPIO_Init();

    /* USART1'i 115200 baud, 8N1 modunda başlat */
    MX_USART1_UART_Init();

    /* Ana sonsuz döngü */
    while (1)
    {
        /* UART üzerinden tam olarak 2 byte bekle.
           HAL_UART_Receive bloklayan moddadır: veri gelene ya da 1000 ms
           timeout dolana kadar burada bekler. */
        if (HAL_UART_Receive(&huart1, rxBuffer, 2, 1000) == HAL_OK)
        {
            /* 2 byte başarıyla alındı → mesajı işle */
            processUARTMessage(rxBuffer);
        }
        /* HAL_TIMEOUT döndürdüyse yeni mesaj beklemeye devam et */
    }
}

/* ==================== MESAJ İŞLEYİCİ ==================== */

/**
 * @brief  Alınan 2 karakterlik UART mesajına göre PA0 ve PA1 çıkışlarını ayarla.
 * @param  buffer  2 byte'lık mesaj tamponu (buffer[0]=Pin1 komutu, buffer[1]=Pin2 komutu)
 * @retval None
 */
void processUARTMessage(uint8_t *buffer)
{
    /* ---- İLK KARAKTER → PA0 (GPIO Pin 1) ---- */
    switch (buffer[0])
    {
        case '0':
            /* '0': PA0'ı LOW yap (lojik sıfır, 0V) */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
            break;

        case '1':
            /* '1': PA0'ı HIGH yap (lojik bir, 3.3V) */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
            break;

        case '2':
            /* '2': PA0'ı TOGGLE yap (HIGH ise LOW, LOW ise HIGH) */
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
            break;

        default:
            /* Tanımsız karakter: PA0'a dokunma, güvenli kalın */
            break;
    }

    /* ---- İKİNCİ KARAKTER → PA1 (GPIO Pin 2) ---- */
    switch (buffer[1])
    {
        case '0':
            /* '0': PA1'i LOW yap */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
            break;

        case '1':
            /* '1': PA1'i HIGH yap */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
            break;

        case '2':
            /* '2': PA1'i TOGGLE yap */
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
            break;

        default:
            /* Tanımsız karakter: PA1'e dokunma */
            break;
    }
}

/* ==================== DONANIM BAŞLATMA FONKSİYONLARI ==================== */

/**
 * @brief  USART1 UART çevre birimini başlat.
 *         TX: PA9, RX: PA10 (STM32F103C8T6 varsayılan pin eşlemesi)
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;                        /* Hangi USART? → USART1 */
    huart1.Init.BaudRate = 115200;                   /* Seri hız: 115200 bps */
    huart1.Init.WordLength = UART_WORDLENGTH_8B;     /* Veri biti sayısı: 8 */
    huart1.Init.StopBits = UART_STOPBITS_1;          /* Stop bit: 1 */
    huart1.Init.Parity = UART_PARITY_NONE;           /* Parity kontrolü: yok */
    huart1.Init.Mode = UART_MODE_TX_RX;              /* Hem alım hem gönderim aktif */
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;     /* Donanım akış kontrolü: yok */
    huart1.Init.OverSampling = UART_OVERSAMPLING_16; /* Gürültü direnci için 16x örnekleme */

    /* HAL_UART_Init başarısız olursa sonsuz döngüde bekle (hata ayıklama kolaylığı) */
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  GPIO pinlerini (PA0, PA1) dijital çıkış olarak başlat.
 *         Başlangıç durumu: LOW (her iki pin de 0V) — güvenli varsayılan.
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0}; /* Yapıyı sıfırla — eski değerleri önle */

    /* GPIOA periferik saatini etkinleştir.
       Saat verilmeden GPIO registerlarına yazmak çalışmaz. */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* PA0 ve PA1'i başlangıçta LOW olarak ayarla.
       Init fonksiyonu çağrılmadan önce çıkış durumunu belirlemek
       açılışta istenmeyen HIGH pulse'larını önler. */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_RESET);

    /* PA0 ve PA1 pin konfigürasyonu */
    GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_1; /* Aynı anda iki pini yapılandır */
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;      /* Push-Pull çıkış (aktif sürücü) */
    GPIO_InitStruct.Pull  = GPIO_NOPULL;              /* Dahili pull-up/pull-down devre dışı */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;      /* 2 MHz — GPIO toggle için yeterli */

    /* Yapılandırmayı GPIOA registerlarına yaz */
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief  Sistem saatini yapılandır.
 *         HSE (8 MHz harici kristal) → PLL × 9 = 72 MHz SYSCLK
 *         APB1 = 36 MHz, APB2 = 72 MHz
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* HSE osilatörü etkinleştir ve PLL'i HSE × 9 = 72 MHz'e ayarla */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState            = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue      = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL9; /* 8 MHz × 9 = 72 MHz */

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Sistem saati kaynaklarını seç ve bus prescaler'larını ayarla */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK |
                                       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK; /* PLL → SYSCLK */
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;          /* HCLK  = 72 MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;            /* APB1  = 36 MHz (max) */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;            /* APB2  = 72 MHz */

    /* Flash gecikme: 72 MHz için 2 wait state gerekli */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ==================== HATA İŞLEYİCİ ==================== */

/**
 * @brief  Kritik hata durumunda çağrılır; interrupt'ları devre dışı bırakıp
 *         sonsuz döngüde bekler (debugger ile yakalanabilir).
 * @retval None
 */
void Error_Handler(void)
{
    __disable_irq(); /* Tüm kesmeleri devre dışı bırak */
    while (1)
    {
        /* Hata durumu: sonsuz döngü.
           Debugger bağlıysa bu noktada pause yaparak kaynağı inceleyebilirsiniz. */
    }
}
