# Guia de montagem física, passo a passo

Monte **uma peça de cada vez** e teste antes de passar para a próxima.
Assim, se algo der errado, sabemos exatamente onde está o problema.

> ⚠️ **Regra de ouro:** sempre **desligue o USB** antes de mexer em qualquer fio.

---

## Passo 0: Preparar o computador

1. Baixe e instale o **Arduino IDE** em arduino.cc/en/software.
2. Abra o programa. Na barra da esquerda, clique no ícone de **livros** 📚 (Library Manager).
3. Pesquise e instale:
   - **LiquidCrystal I2C**, de *Frank de Brabander*
   - **MFRC522**, de *GithubCommunity*
   - **Servo**, de *Arduino* (normalmente já vem instalada)
4. Ligue o Arduino no USB.
5. Em cima, selecione a placa **Arduino Uno** e a porta (COM3, COM4… no Windows).

**Para enviar um programa:** cole o código, clique na **seta →** (Upload) e espere "Done uploading".
**Para ver o diário (Monitor Serial):** ícone de **lupa** 🔍 no canto superior direito, velocidade **9600**.

---

## Passo 1: Trilhos de energia da protoboard

A protoboard tem nas bordas duas linhas compridas: **vermelha (+)** e **azul (−)**.
Vamos usá-las para distribuir energia, porque o Arduino tem só um pino de 5V.

| Fio | De | Para |
|---|---|---|
| 1 | Arduino **5V** | linha **vermelha (+)** da protoboard |
| 2 | Arduino **GND** | linha **azul (−)** da protoboard |

> A partir daqui, "**+5V**" significa a linha vermelha e "**GND**" significa a linha azul.

---

## Passo 2: Display LCD

### Ligações (4 fios macho-fêmea)
| Pino do display (plaquinha preta atrás) | Vai para |
|---|---|
| **GND** | GND (linha azul) |
| **VCC** | +5V (linha vermelha) |
| **SDA** | Arduino **A4** |
| **SCL** | Arduino **A5** |

### Teste
1. Abra `testes/Teste1_LCD/Teste1_LCD.ino`, copie, cole no Arduino IDE e envie.
2. O display deve mostrar **"SmartAccess"** com um contador e **"Teste LCD OK!"**.
3. No Monitor Serial aparece o **endereço** do display (ex.: `0x27`). **Anote:** ________

### Problemas
| Sintoma | Solução |
|---|---|
| Luz azul acesa, sem letras | Girar devagar o **parafusinho do quadrado azul** atrás do display (contraste) |
| "nenhum display encontrado" | Conferir os 4 fios; SDA e SCL trocados é o erro mais comum |
| Display apagado | Conferir VCC/GND e o **jumper preto** no canto da plaquinha (liga a luz de fundo) |

---

## Passo 3: Leitor RFID RC522

### Ligações (7 fios macho-fêmea)
Os nomes estão escritos ao lado dos pinos do leitor.

| Pino do RC522 | Vai para |
|---|---|
| **SDA** | Arduino **10** |
| **SCK** | Arduino **13** |
| **MOSI** | Arduino **11** |
| **MISO** | Arduino **12** |
| **IRQ** | *(não ligar)* |
| **GND** | GND (linha azul) |
| **RST** | Arduino **9** |
| **3.3V** | Arduino **3.3V** ⚠️ |

> ⚠️ **O 3.3V do leitor vai no pino "3.3V" do Arduino, NUNCA no 5V.** No 5V o leitor queima.
>
> Os pinos de dados do Arduino trabalham em 5V e o leitor em 3,3V. O jeito mais seguro seria um
> conversor de nível lógico, mas os kits de laboratório normalmente são usados ligando direto,
> e costuma funcionar. Se o laboratório tiver um conversor de nível, melhor ainda.

### Teste e descobrir os números dos chaveiros
1. Abra `testes/Teste2_RFID/Teste2_RFID.ino`, copie, cole e envie.
2. Abra o Monitor Serial (9600). Deve aparecer **"Leitor OK!"**.
3. Encoste **cada chaveiro**, um de cada vez, e anote o número de cada um:

| Chaveiro | Função | UID (o que aparece) | Formato para o código |
|---|---|---|---|
| Nº 1 (marcar com caneta) | 👑 **Chefe (admin)** | | `{0x__, 0x__, 0x__, 0x__}` |
| Nº 2 | ✅ Autorizado | | |
| Nº 3 | ❌ Desconhecido | | |

> Todos precisam mostrar **"4 bytes"**. Se algum mostrar 7 bytes, avise que adaptamos o código.

### Problemas
| Sintoma | Solução |
|---|---|
| "Versao 0x0" ou "0xFF" / "nao respondeu" | Conferir os 7 fios (principalmente SDA no 10 e RST no 9) e o 3.3V |
| Leitor OK, mas não lê o chaveiro | Encostar o chaveiro bem no centro do desenho de ondas |

---

## Passo 4: Servo motor (a trava)

### Ligações (3 fios)
O servo tem 3 fios coloridos num conector preto:

| Fio do servo | Vai para |
|---|---|
| **Marrom** | GND (linha azul) |
| **Vermelho** | +5V (linha vermelha) |
| **Laranja** | Arduino **3** |

> Se o Arduino **reiniciar sozinho** quando o servo gira, o USB não está dando energia suficiente.
> Solução: alimentar o servo com pilhas/fonte de 5V separada, **ligando o GND da fonte junto com o GND do Arduino**.

O braço branco do servo pode ser preso com fita a um palito de picolé, que vira a "lingueta" da trava.

---

## Passo 5: Buzzer

| Perninha | Vai para |
|---|---|
| Perninha **mais comprida** (ou marcada com **+**) | Arduino **4** |
| Outra perninha | GND (linha azul) |

---

## Passo 6: LEDs (opcional)

### Opção A: LED vermelho e verde comuns (2 perninhas)
| Ligação | |
|---|---|
| Arduino **5** → resistor → perninha **comprida** do LED **verde** | perninha curta → GND |
| Arduino **6** → resistor → perninha **comprida** do LED **vermelho** | perninha curta → GND |

No código: `LED_ANODO_COMUM = false`.

### Opção B: LED RGB (4 perninhas, branco/fosco)
A perninha **mais comprida** é a "comum". A ordem normal é: **Vermelho – Comum – Verde – Azul**.

**Descobrir o tipo:** ligue a perninha comprida no **GND** e encoste a perninha do vermelho no
**+5V passando por um resistor**.
- **Acendeu vermelho** → é **catodo comum**. Perninha comprida no GND; no código `LED_ANODO_COMUM = false`.
- **Não acendeu** → inverta: perninha comprida no **+5V** e a outra no GND pelo resistor. Se acender,
  é **anodo comum**: perninha comprida no +5V; no código `LED_ANODO_COMUM = true`.

| Perninha do RGB | Vai para |
|---|---|
| Vermelho | resistor → Arduino **6** |
| Comprida (comum) | GND (catodo) **ou** +5V (anodo) |
| Verde | resistor → Arduino **5** |
| Azul | *(não usar)* |

### Sem LEDs
Tudo bem: o projeto funciona sem eles (veja `INVENTARIO.md`).

---

## Passo 7: Botão (opcional)

| Ligação | |
|---|---|
| Uma perninha do botão | Arduino **7** |
| Perninha do **lado oposto (na diagonal)** | GND (linha azul) |

**Sem botão:** um fio no pino **7** e outro no **GND**. Encostar um no outro = apertar o botão.

---

## Passo 8: Programa final

1. Abra `firmware/SmartAccess/SmartAccess.ino`.
2. Mude estas linhas no começo do código:

```cpp
#define MODO_SIMULACAO 0          // 0 = leitor RFID de verdade

const byte ENDERECO_LCD = 0x27;   // o endereço anotado no Passo 2

const byte UID_ADMIN[4] = {0x__, 0x__, 0x__, 0x__};  // chaveiro do CHEFE (Passo 3)

const bool LED_ANODO_COMUM = false;  // true só se o LED RGB for anodo comum (Passo 6)
```

3. Envie para o Arduino.
4. **Cadastrar o chaveiro autorizado:**
   1. Encoste o chaveiro do **chefe** → "ADMIN: CADASTRAR"
   2. Encoste o chaveiro **nº 2** → "CADASTRADO!"
   3. Encoste o chaveiro do **chefe** de novo → sai do menu
5. Teste: o nº 2 abre a porta e o nº 3 é negado.

---

## Passo 9: Testes finais (marcar antes da apresentação)

| # | Teste | Esperado | OK? |
|---|---|---|---|
| T01 | Ligar | Tela inicial, servo fechado | [ ] |
| T02 | Chaveiro autorizado | LIBERADO, bip agudo, servo abre e fecha após 4 s | [ ] |
| T03 | Chaveiro desconhecido | NEGADO, bip grave, servo não mexe | [ ] |
| T04 | Desconhecido 3 vezes | BLOQUEADO com contagem de 30 s | [ ] |
| T05 | Autorizado durante o bloqueio | Não abre | [ ] |
| T06 | Esperar 30 s | Volta à tela inicial | [ ] |
| T07 | Chaveiro do chefe | Menu "ADMIN: CADASTRAR" | [ ] |
| T08 | Cadastrar o nº 3 | Passa a abrir a porta | [ ] |
| T09 | Desligar e religar o USB | O nº 3 continua abrindo (EEPROM) | [ ] |
| T10 | Chefe → botão (EXCLUIR) → nº 3 | O nº 3 volta a ser negado | [ ] |
| T11 | Segurar o botão 3 s | Entra no menu admin | [ ] |
| T12 | Abrir e fechar 10 vezes | Nada trava nem reinicia | [ ] |

---

## Mapa completo de ligações

| Componente | Pino do componente | Arduino / Protoboard |
|---|---|---|
| Protoboard | linha vermelha (+) | 5V |
| Protoboard | linha azul (−) | GND |
| LCD | GND / VCC / SDA / SCL | GND / +5V / A4 / A5 |
| RC522 | SDA / SCK / MOSI / MISO | 10 / 13 / 11 / 12 |
| RC522 | RST / GND / 3.3V | 9 / GND / **3.3V** |
| Servo | marrom / vermelho / laranja | GND / +5V / 3 |
| Buzzer | + / − | 4 / GND |
| LED verde | via resistor | 5 |
| LED vermelho | via resistor | 6 |
| Botão | um lado / lado oposto | 7 / GND |
