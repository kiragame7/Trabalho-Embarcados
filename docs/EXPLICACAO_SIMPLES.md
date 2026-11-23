# SmartAccess explicado de forma simples

Leia este arquivo primeiro. Ele explica o projeto sem termos técnicos.
Também serve para estudar para a apresentação.

---

## 1. Qual é a ideia do projeto?

Pense na **catraca de uma academia** ou na **porta de um prédio que abre com cartão**.

Vamos montar uma **portinha** que só abre quando alguém encosta o **chaveiro certo** num leitor.

- Chaveiro certo → a porta abre
- Chaveiro errado → a porta não abre e aparece um aviso
- 3 tentativas erradas seguidas → o sistema trava por 30 segundos, contra quem fica tentando adivinhar

---

## 2. As peças são como um corpo humano

| Peça | Comparação | O que faz |
|---|---|---|
| **Arduino Uno** | 🧠 Cérebro | Recebe as informações e decide o que fazer |
| **Leitor RFID (RC522)** | 👀 Olhos | "Vê" o chaveiro quando ele chega perto |
| **Display LCD** | 🗣️ Boca | Mostra mensagens como "ACESSO LIBERADO" |
| **Servo motor** | 💪 Braço | Gira e destrava a porta |
| **LED verde/vermelho** | 🚦 Semáforo | Verde = pode passar, vermelho = não pode |
| **Buzzer** | 🔔 Campainha | Faz "bip": agudo = liberado, grave = negado |
| **Botão** | 👆 Interruptor | O administrador usa para trocar de opção no menu |
| **Memória EEPROM** | 📓 Caderninho | Guarda a lista de chaveiros permitidos, mesmo desligado |

---

## 3. Como o Arduino sabe qual chaveiro é qual?

Todo chaveiro ou cartão RFID tem um **número de identidade gravado dentro**, como se fosse o
**CPF do cartão**. Esse número se chama **UID**. Exemplo: `12 AB 34 CD`.

O Arduino guarda no "caderninho" a **lista de números permitidos**. Quando um chaveiro chega
perto, ele lê o número e procura na lista:
- **está na lista** → abre;
- **não está** → nega.

---

## 4. Passo a passo do que acontece

1. **Ligou:** a tela mostra "SMARTACCESS / Aproxime cartao" e ele fica esperando.
2. **Alguém encosta o chaveiro:** o leitor lê o número dele.
3. **O Arduino procura** esse número no caderninho.
4. **Se achou:** tela "ACESSO LIBERADO", LED verde, bip agudo, o servo gira e a porta abre.
   Depois de 4 segundos a porta fecha sozinha.
5. **Se não achou:** tela "ACESSO NEGADO", LED vermelho, bip grave, a porta continua fechada.
6. **Se errar 3 vezes seguidas:** tela "BLOQUEADO!" com contagem de 30 até 0.
   Nesse tempo, nem o chaveiro certo funciona.

---

## 5. O administrador ("o chefe")

Existe um **chaveiro do chefe**. Ele não abre a porta: abre um **menu para mexer na lista**.

- Encostou o chaveiro do chefe → a tela mostra **"ADMIN: CADASTRAR"**.
- Encostou outro chaveiro → esse chaveiro **entra na lista** e passa a abrir a porta.
- Apertou o botão → muda para **"ADMIN: EXCLUIR"**. O próximo chaveiro encostado **sai da lista**.
- Encostou o chaveiro do chefe de novo → sai do menu.
- Ficou 15 segundos sem fazer nada → sai do menu sozinho.

**Outra forma de entrar no menu:** **segurar o botão por 3 segundos**. Serve caso não haja
chaveiros suficientes.

Assim dá para dar e tirar permissão de pessoas **sem mexer no código**.

**Apagar tudo:** segurar o botão enquanto liga o Arduino apaga todos os cadastros.

---

## 6. O que é o Wokwi?

É um **site que simula o Arduino**, como um ensaio antes de montar de verdade.
Como o Wokwi não tem o leitor de cartão, usamos **botões coloridos que fingem ser chaveiros**:

| Botão no Wokwi | Representa |
|---|---|
| 🟢 Verde | Cartão 1 (já está na lista) |
| ⚪ Cinza | Cartão 2 (começa fora da lista) |
| 🔵 Azul | Chaveiro do chefe (admin) |
| 🟡 Amarelo | O botão de verdade (troca CADASTRAR/EXCLUIR; segurar 3 s entra/sai do menu) |

O texto que aparece embaixo, no **Monitor Serial**, é o **"diário" do Arduino**: ele anota tudo o que
acontece. Não precisa digitar nada lá.

---

## 7. Palavras técnicas que o professor pode perguntar

| Palavra | Explicação simples |
|---|---|
| **RFID** | Identificação por rádio: o leitor "conversa" com o chaveiro sem encostar, por ondas de rádio |
| **UID** | O "CPF" do chaveiro, um número único gravado nele |
| **SPI** | O "idioma" que o leitor RFID usa para conversar com o Arduino (4 fios de dados) |
| **I2C** | O "idioma" que o display usa (só 2 fios de dados: SDA e SCL) |
| **EEPROM** | Memória do Arduino que **não apaga** quando desliga (o caderninho) |
| **Servo** | Motor que gira até um ângulo exato (0° = fechado, 90° = aberto) |
| **Entrada digital** | Algo que o Arduino **lê** como ligado/desligado (o botão) |
| **Saída digital** | Algo que o Arduino **liga/desliga** (LED, buzzer) |
| **millis()** | O "relógio" do Arduino; usamos para contar tempo sem travar o programa |
| **Máquina de estados** | O sistema sempre está em **um** modo: esperando, porta aberta, negado, bloqueado ou admin |

### Perguntas prováveis

- **Por que RFID?** Permite identificar pessoas sem contato e mostra comunicação SPI.
- **Como sabe se o cartão é válido?** Lê o UID e compara com a lista salva na EEPROM.
- **Por que `millis()` e não `delay()`?** O `delay()` congela o Arduino; com `millis()` ele continua
  lendo cartões e o botão enquanto conta o tempo.
- **Diferença entre SPI e I2C?** SPI usa mais fios e é mais rápido; I2C usa só 2 fios
  compartilhados, e cada aparelho tem um "endereço" (o display é 0x27).
- **É seguro de verdade?** Não. O UID pode ser copiado. É um protótipo para aprender; um sistema
  real usaria criptografia.
- **O que melhorariam?** Criptografia, sensor de porta aberta, registro de horários de entrada (relógio),
  aviso pelo celular.

---

## 8. Etapas do projeto

```
[✅ Ideia] → [✅ Código] → [✅ Simulação Wokwi] → [⏳ Juntar peças] → [Montar físico] → [Apresentar 30/11]
```
