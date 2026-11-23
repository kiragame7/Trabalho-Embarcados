# SmartAccess 🔐

Sistema de controle de acesso com Arduino Uno e RFID: projeto final de **Sistemas Embarcados**
(Prof. Cleber Lourenço Izidoro). **Apresentação: 30/11.**

Um chaveiro RFID é aproximado do leitor RC522. O Arduino verifica se o número (UID) está
cadastrado na EEPROM, mostra o resultado no LCD 16x2 (I2C), abre a trava (servo) e sinaliza
com LED e buzzer. Três tentativas inválidas seguidas bloqueiam o sistema por 30 s. Um chaveiro
administrador (ou segurar o botão por 3 s) abre o menu para cadastrar ou excluir chaveiros.

---

## 📍 Por onde começar

| Ordem | Arquivo | Para quê |
|---:|---|---|
| 1 | [`docs/EXPLICACAO_SIMPLES.md`](docs/EXPLICACAO_SIMPLES.md) | **Entender o projeto** sem termos técnicos + perguntas do professor |
| 2 | [`docs/INVENTARIO.md`](docs/INVENTARIO.md) | O que já temos, o que falta, o que **não** serve |
| 3 | [`docs/GUIA_WOKWI.md`](docs/GUIA_WOKWI.md) | Simulação no site Wokwi (já funcionou ✅) |
| 4 | [`docs/GUIA_MONTAGEM_FISICA.md`](docs/GUIA_MONTAGEM_FISICA.md) | **Montagem real, passo a passo**, com testes |
| 5 | [`docs/LISTA_COMPONENTES.md`](docs/LISTA_COMPONENTES.md) | Lista de compras original |
| — | [`docs/SmartAccess_Guia_Completo.md`](docs/SmartAccess_Guia_Completo.md) | Guia geral (slides, roteiro da apresentação, cronograma) |

## 📁 Código

| Arquivo | O que é |
|---|---|
| `firmware/SmartAccess/SmartAccess.ino` | **Programa final**. `MODO_SIMULACAO 1` = Wokwi, `0` = físico |
| `testes/Teste1_LCD/Teste1_LCD.ino` | Teste só do display (descobre o endereço I2C) |
| `testes/Teste2_RFID/Teste2_RFID.ino` | Teste só do leitor (mostra o UID de cada chaveiro) |
| `wokwi/diagram.json` | Circuito para colar no Wokwi |
| `wokwi/libraries.txt` | Bibliotecas do Wokwi |

---

## ✅ Andamento

- [x] Ideia validada com os requisitos do PDF
- [x] Código completo
- [x] Simulação no Wokwi funcionando
- [x] Peças conferidas: Arduino, protoboard, display I2C, leitor RFID (com pinos soldados), servo, buzzer, chaveiros
- [ ] Conseguir **fios jumper** (macho-fêmea e macho-macho) e cabo USB
- [ ] (Opcional) LED RGB ou LEDs verde/vermelho + resistores + botão
- [ ] Passo 2: display funcionando (`Teste1_LCD`)
- [ ] Passo 3: leitor funcionando e UIDs anotados (`Teste2_RFID`)
- [ ] Passos 4–7: servo, buzzer, LEDs, botão
- [ ] Passo 8: programa final no físico (`MODO_SIMULACAO 0`)
- [ ] Testes T01–T12
- [ ] Maquete (portinha)
- [ ] Slides + ensaio da apresentação
- [ ] Vídeo curto de backup da demonstração

---

## Requisitos da disciplina × projeto

| Requisito do PDF | Como o SmartAccess atende |
|---|---|
| Situação-problema | Controle de acesso a laboratórios/salas restritas sem chaves físicas |
| Entradas digitais | Botão MODO (D7, `INPUT_PULLUP`) |
| Saídas digitais | LED verde (D5), LED vermelho (D6), buzzer (D4), sinal do servo (D3) |
| Pelo menos 1 display | LCD 16x2 com módulo I2C |
| Recurso novo (sensor/atuador/comunicação) | Leitor RFID RC522 via **SPI**, LCD via **I2C**, **servo**, **EEPROM** |
| Simulador (opcional) | Simulação completa no Wokwi |

**Avaliação:** criatividade 1,0 · apresentação oral 2,0 · estrutura da programação 2,0 ·
**funcionamento 4,0** · respostas às perguntas 1,0.

## Pinagem

| Componente | Arduino Uno |
|---|---|
| RC522 SDA, SCK, MOSI, MISO, RST | 10, 13, 11, 12, 9 |
| RC522 alimentação | **3.3V** (nunca 5V!) |
| LCD I2C SDA / SCL | A4 / A5 |
| Servo (sinal, fio laranja) | 3 |
| Buzzer | 4 |
| LED verde / vermelho (ou RGB) | 5 / 6 (com resistor) |
| Botão MODO | 7 → GND |
| *(só simulação)* botões Admin / Cartão 1 / Cartão 2 | A0 / A1 / A2 → GND |

## Organização do código

- **Configuração** no topo: pinos, tempos, ângulos, UID do admin, tipo de LED.
- **Máquina de estados**: `AGUARDANDO`, `PORTA_ABERTA`, `ACESSO_NEGADO`, `BLOQUEADO`, `MODO_ADMIN`.
- **Sem `delay()` no loop**: todos os tempos usam `millis()`, inclusive o debounce dos botões.
- **Camada de leitura** (`lerCartao`): RC522 no físico; botões/Serial na simulação. O resto do
  código é idêntico nos dois modos.
- **EEPROM**: assinatura + quantidade + até 15 UIDs de 4 bytes; `EEPROM.update()` evita gravações
  desnecessárias.
- **Botão MODO**: toque = alterna cadastrar/excluir; segurar 3 s = entra/sai do admin;
  segurar ao ligar = apaga todos os cadastros.
- Monitor Serial (9600): comandos `LISTA` e `AJUDA`.

> Protótipo didático: identificação só por UID pode ser clonada. Não usar como fechadura real.
