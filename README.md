# SmartAccess 🔐

Sistema de controle de acesso com Arduino Uno e RFID: projeto final de **Sistemas Embarcados**.

Um cartão RFID é aproximado do leitor RC522. O Arduino verifica se o UID está cadastrado na
EEPROM, mostra o resultado no LCD 16x2 (I2C), abre a trava (servo) e sinaliza com LEDs e buzzer.
Três tentativas inválidas seguidas bloqueiam o sistema por 30 s. Um cartão administrador
abre o menu para cadastrar ou excluir cartões.

## Estrutura

| Caminho | Conteúdo |
|---|---|
| `firmware/SmartAccess/SmartAccess.ino` | Firmware (simulação e físico, escolhido por `MODO_SIMULACAO`) |
| `wokwi/diagram.json` | Circuito pronto para colar no Wokwi |
| `wokwi/libraries.txt` | Bibliotecas do Wokwi |
| `docs/GUIA_WOKWI.md` | Passo a passo da simulação e roteiro de testes |
| `docs/SmartAccess_Guia_Completo.md` | Guia geral do projeto (montagem, apresentação, perguntas) |

## Requisitos da disciplina × projeto

| Requisito do PDF | Como o SmartAccess atende |
|---|---|
| Situação-problema | Controle de acesso a laboratórios/salas restritas sem chaves físicas |
| Entradas digitais | Botão MODO (D7, `INPUT_PULLUP`) |
| Saídas digitais | LED verde (D5), LED vermelho (D6), buzzer (D4), sinal do servo (D3) |
| Pelo menos 1 display | LCD 16x2 com módulo I2C |
| Recurso novo (sensor/atuador/comunicação) | Leitor RFID RC522 via **SPI**, LCD via **I2C**, **servo**, **EEPROM** |
| Simulador (opcional) | Simulação completa no Wokwi |

## Pinagem

| Componente | Arduino Uno |
|---|---|
| RC522 SDA/SS, SCK, MOSI, MISO, RST | D10, D13, D11, D12, D9 (3,3 V!) |
| LCD I2C SDA / SCL | A4 / A5 |
| Servo (sinal) | D3 |
| Buzzer | D4 |
| LED verde / vermelho | D5 / D6 (com resistor 220 Ω) |
| Botão MODO | D7 → GND |
| *(só simulação)* botões Admin / Cartão 1 / Cartão 2 | A0 / A1 / A2 → GND |

## Organização do código

- **Configuração** no topo: pinos, tempos, ângulos, UID do admin.
- **Máquina de estados**: `AGUARDANDO`, `PORTA_ABERTA`, `ACESSO_NEGADO`, `BLOQUEADO`, `MODO_ADMIN`.
- **Sem `delay()` no loop**: todos os tempos usam `millis()`, inclusive o debounce dos botões.
- **Camada de leitura** (`lerCartao`): RC522 no físico; botões/Serial na simulação. O resto
  do código é o mesmo nos dois modos.
- **EEPROM**: assinatura + quantidade + até 15 UIDs de 4 bytes; `EEPROM.update()` evita
  gravações desnecessárias.
- Segurar o botão **MODO ao ligar** apaga todos os cadastros.

> Protótipo didático: identificação só por UID pode ser clonada. Não usar como fechadura real.
