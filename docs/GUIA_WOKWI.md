# Guia: montando o SmartAccess no Wokwi

O Wokwi **não tem o leitor RFID RC522** na lista de componentes. Por isso o firmware tem
um `MODO_SIMULACAO`: os cartões são simulados por **botões** e pelo **Monitor Serial**.
O resto do circuito (LCD I2C, servo, LEDs, buzzer e botão MODO) é igual ao físico.

## Passo 1 – Criar o projeto

1. Acesse <https://wokwi.com> e faça login (dá para salvar o projeto e compartilhar o link).
2. Clique em **New Project → Arduino Uno**.
3. Vão aparecer três abas: `sketch.ino`, `diagram.json` e o simulador.

## Passo 2 – Colar o código

1. Abra `firmware/SmartAccess/SmartAccess.ino` deste repositório.
2. Confira que a linha está assim: `#define MODO_SIMULACAO 1`
3. Apague tudo da aba `sketch.ino` do Wokwi e cole o código.

## Passo 3 – Colar o circuito

1. Abra a aba **diagram.json** no Wokwi.
2. Apague tudo e cole o conteúdo de `wokwi/diagram.json`.
3. O circuito aparece montado. Os fios ficam retos, e dá para arrastar as peças para organizar.

## Passo 4 – Adicionar as bibliotecas

Na aba de arquivos, clique em **Library Manager** (ou no botão `+` → *New file* → `libraries.txt`)
e adicione:

- `LiquidCrystal I2C`
- `Servo`

(Ou crie o arquivo `libraries.txt` com o conteúdo de `wokwi/libraries.txt`.)

## Passo 5 – Rodar

Clique no botão verde ▶ (**Start simulation**). O LCD deve mostrar:

```
  SMARTACCESS
Aproxime cartao
```

### Controles da simulação

| Botão no circuito | Tecla | Representa |
|---|---|---|
| Azul | `1` | Aproximar o **cartão ADMIN** (UID `DE AD BE EF`) |
| Verde | `2` | Aproximar o **Cartão 1** (UID `11 22 33 44`) – **já vem cadastrado** |
| Cinza | `3` | Aproximar o **Cartão 2** (UID `55 66 77 88`) – **começa desconhecido** |
| Amarelo | `M` | Botão **MODO** (alterna CADASTRAR/EXCLUIR no menu admin) |

> As teclas só funcionam depois de clicar uma vez em cima do simulador.

No **Monitor Serial** (aparece embaixo do simulador) também dá para:

- digitar um UID qualquer, ex. `A1 B2 C3 D4` + Enter → simula um cartão com esse UID;
- digitar `LISTA` → mostra os cartões cadastrados;
- digitar `AJUDA` → mostra os comandos.

## Passo 6 – Roteiro de testes no Wokwi

Façam na ordem e marquem o resultado:

| # | Ação | Resultado esperado | OK? |
|---|---|---|---|
| W1 | Iniciar simulação | LCD "SMARTACCESS / Aproxime cartao", servo em 0° | [ ] |
| W2 | Botão **verde** (Cartão 1) | "ACESSO LIBERADO", LED verde, bip agudo, servo vai a 90° e volta após 4 s | [ ] |
| W3 | Botão **cinza** (Cartão 2) | "ACESSO NEGADO / Tentativa 1/3", LED vermelho, bip grave | [ ] |
| W4 | Cinza mais 2 vezes (esperar a tela voltar entre elas) | "BLOQUEADO!" com contagem 30 → 0 s, LED vermelho piscando | [ ] |
| W5 | Botão verde durante o bloqueio | Não abre (Serial: "Ignorado: sistema bloqueado") | [ ] |
| W6 | Esperar o fim do bloqueio | Volta para a tela inicial | [ ] |
| W7 | Botão **azul** (Admin) | "ADMIN: CADASTRAR / Cartoes: 01/15", LED verde piscando | [ ] |
| W8 | Botão cinza (Cartão 2) | "CADASTRADO! / Total: 02/15", depois volta ao menu | [ ] |
| W9 | Botão azul (sair) e depois cinza | Cartão 2 agora **abre** a porta | [ ] |
| W10 | Azul → **amarelo** (MODO) → verde | "ADMIN: EXCLUIR", depois "EXCLUIDO!" | [ ] |
| W11 | Sair do admin e apertar verde | Cartão 1 agora é **negado** | [ ] |
| W12 | Entrar no admin e esperar 15 s | Sai sozinho do menu (tempo esgotado) | [ ] |
| W13 | Serial: digitar `LISTA` | Lista de UIDs cadastrados | [ ] |

### Sobre a EEPROM no Wokwi

O Wokwi normalmente **zera a EEPROM a cada nova simulação**, então o teste "desligar e
religar e o cadastro continua" deve ser feito **no circuito físico**. Na simulação, toda vez
que inicia a memória é formatada e o Cartão 1 é cadastrado de novo automaticamente.

## Problemas comuns

| Problema | Solução |
|---|---|
| Erro `LiquidCrystal_I2C.h: No such file` | Adicionar a biblioteca **LiquidCrystal I2C** (Passo 4) |
| Erro `MFRC522.h: No such file` | `MODO_SIMULACAO` está em `0`; mudar para `1` no Wokwi |
| LCD em branco | Conferir `"pins": "i2c"` no diagram.json e `ENDERECO_LCD = 0x27` |
| Botão não responde | Conferir se o fio vai para o pino certo e o outro lado para GND |
| Teclas 1/2/3/M não funcionam | Clicar no simulador antes de usar o teclado |

## Depois do Wokwi → circuito físico

No circuito real só muda uma linha:

```cpp
#define MODO_SIMULACAO 0
```

Com isso o código passa a usar o leitor RC522 (SPI), e os botões de "cartão" (A0–A2)
deixam de existir. Também é preciso trocar `UID_ADMIN` pelo UID do cartão real, que aparece
no Monitor Serial na primeira leitura (veja a seção 6.1 do guia completo).
