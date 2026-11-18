# SempreAqui

Sistema de monitoramento de baixo custo para pessoas com Alzheimer. Ele avisa o cuidador quando o paciente se afasta da área segura da casa sem autorização.

Projeto desenvolvido para o TDE da disciplina **Conectividade em Sistemas Ciberfísicos**, do curso de Engenharia de Software da **PUCPR Toledo** (2026), pelo grupo **Cabo Sem Fio**.

## Como funciona

O sistema tem duas partes:

- **Pulseira** (ESP32-C3 SuperMini): fica com o paciente e emite continuamente um sinal **Bluetooth Low Energy (BLE)** com o nome `SempreAqui`.
- **Central** (ESP32 na placa ES32Lab): fica dentro da casa. Ela procura o sinal da pulseira e mede a intensidade dele (**RSSI**). Quanto mais forte o sinal, mais perto está o paciente.

A comunicação é direta entre as duas placas, sem roteador, internet ou GPS.

Para evitar que a variação natural do sinal faça os LEDs trocarem de cor sozinhos, a central usa a média das leituras (`média = 0,7 × média anterior + 0,3 × leitura nova`) e uma folga de 3 dBm entre os estados "perto" e "longe".

### Estados da central

| LED | Estado | Quando ocorre |
|---|---|---|
| 🟢 Verde | Pulseira perto | Média do sinal acima de −75 dBm (cerca de 2 m) |
| 🟡 Amarelo | Saída liberada | Cartão autorizado passado no leitor RFID |
| 🔴 Vermelho | Pulseira longe | Média abaixo de −78 dBm ou 5 s sem sinal |

### Leitor RFID (porta)

- O **primeiro cartão** passado é cadastrado automaticamente e fica salvo na memória flash da central, mesmo desligando.
- Passar o cartão cadastrado **libera a saída** (LED amarelo). Passar de novo **bloqueia**, e a central volta a mostrar a distância.
- Outros cartões são recusados, e o LED vermelho pisca 3 vezes.
- Digitar `c` no Monitor Serial apaga o cartão cadastrado.

O buzzer da ES32Lab é opcional. Para ativá-lo, mude `buzzerLigado` para `true` em `central/central.ino`.

## Estrutura do repositório

```
SempreAqui/
├── central/
│   └── central.ino    # ESP32 (ES32Lab): busca BLE, RSSI, RFID e LEDs
└── pulseira/
    └── pulseira.ino   # ESP32-C3 SuperMini: anúncio BLE
```

## Hardware

**Central**
- Kit ES32Lab (placa ES32Lab + ESP32 DevKit)
- Leitor RFID MFRC522 + cartão
- 3 LEDs (verde, amarelo e vermelho), cada um com um resistor de 220 Ω

**Pulseira**
- ESP32-C3 SuperMini
- LED com resistor de 220 Ω
- Bateria de lítio 3,7 V + módulo carregador TP4056 (USB-C, com proteção)
- Chave liga/desliga

## Ligações

**RFID MFRC522 → ES32Lab**

| MFRC522 | ES32Lab |
|---|---|
| SDA | P04 |
| SCK | P18 |
| MOSI | P23 |
| MISO | P19 |
| IRQ | não ligado |
| GND | GND |
| RST | P14 |
| 3.3V | +3V3 (nunca no 5V) |

**LEDs → ES32Lab** (cada LED com o seu próprio resistor; a perna menor de todos vai no GND)

| LED | Pino |
|---|---|
| Verde | P27 |
| Amarelo | P26 |
| Vermelho | P32 |

**Pulseira**
- LED: pino 0 → resistor → perna maior do LED; perna menor → G
- Bateria → B+ / B- do TP4056
- OUT+ do TP4056 → chave → pino 5V; OUT- → G
- Para carregar, use o USB-C do TP4056. Não ligue o USB da placa com a bateria conectada.

## Como gravar

Pacote de placas: **esp32 (Espressif Systems) 3.3.x**. Biblioteca necessária: **MFRC522** (miguelbalboa). As bibliotecas BLE e Preferences já vêm no pacote do ESP32.

| | Central | Pulseira |
|---|---|---|
| Placa | ESP32 Dev Module | ESP32C3 Dev Module |
| Configuração | Partition Scheme: **Huge APP (3MB No OTA/1MB SPIFFS)** | USB CDC On Boot: **Enabled** · Flash Mode: **DIO** |

## Ajuste da distância

A variável `limiteSinal` em `central/central.ino` define a distância permitida:

| Valor | Distância aproximada |
|---|---|
| `-70` | ~1 m |
| `-75` | ~2 m (padrão) |
| `-80` | ~3 m |

Quanto mais perto de 0, menor a distância. Para calibrar, abra o Monitor Serial da central (115200 baud) e veja a média do sinal na distância desejada.

## Limitações

- O RSSI varia com paredes, móveis e a posição do corpo, então a distância é apenas estimada.
- O alerta é apenas local (LEDs e buzzer).
- Apenas um cartão pode ser cadastrado por vez.

## Equipe — Cabo Sem Fio

- Francisco Henrique Pietrobelli
- Lucas Gabriel Kronbauer
- João Vitor da Silva Souza
- Guilherme Augusto Kist Much
- João Pedro Cordeiro da Silva
- Matheus Nandi
