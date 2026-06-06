# STM32 UART GPIO Kontrolü

STM32F103C8T6 üzerinde UART mesajına göre 2 GPIO pinini (PA0, PA1) kontrol eden gömülü yazılım.

## Mesaj Protokolü

2 karakterlik mesaj gönderilir. İlk karakter PA0'ı, ikinci karakter PA1'i kontrol eder.

| Karakter | Eylem  |
|----------|--------|
| `0`      | LOW    |
| `1`      | HIGH   |
| `2`      | TOGGLE |

**Örnek:** `"21"` → PA0 TOGGLE, PA1 HIGH

## Bağlantılar

| Pin  | Fonksiyon      |
|------|----------------|
| PA10 | USART1 RX      |
| PA9  | USART1 TX      |
| PA0  | GPIO Çıkış 1   |
| PA1  | GPIO Çıkış 2   |

## Seri Port Ayarları

115200 baud, 8N1
