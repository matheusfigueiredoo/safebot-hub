# SafeBot - Sistema de Prevenção de Ciberataques a Robôs

**SafeBot** é um sistema desenvolvido para **prevenir ciberataques** em robôs autônomos e semiautônomos que utilizam o protocolo de comunicação **MQTT**. O sistema tem como objetivo garantir que o robô consiga **detectar e responder** a ameaças em tempo real, mantendo a integridade e a segurança da comunicação entre dispositivos.

---

## 🛠 Tecnologias Usadas

Aqui estão as principais tecnologias e ferramentas utilizadas no desenvolvimento deste projeto:

### Linguagens de Programação

<img src="https://upload.wikimedia.org/wikipedia/commons/6/6a/JavaScript-logo.png" width="50" height="50" />  
JavaScript

<img src="https://w7.pngwing.com/pngs/46/626/png-transparent-c-logo-the-c-programming-language-computer-icons-computer-programming-source-code-programming-miscellaneous-template-blue.png" width="50" height="50" />
C++

### Ambiente de Execução e Web Frameworks

<img src="https://w1.pngwing.com/pngs/885/534/png-transparent-green-grass-nodejs-javascript-react-mean-angularjs-logo-symbol-thumbnail.png" width="50" height="50" />  
Node.js

<img src="https://www.pngfind.com/pngs/m/136-1363736_express-js-icon-png-transparent-png.png" width="50" height="50" />  
Express

### Placa de Desenvolvimento

<img src="https://d229kd5ey79jzj.cloudfront.net/1439/images/1439_3_X.png?20241223122339" width="50" height="50" />  
VESPA (RoboCore)

### Protocolo de Comunicação

<img src="https://automacaoexpert.com/wp-content/uploads/2022/08/MQTT.jpg" width="50" height="50" />  
MQTT

### Broker MQTT

<img src="https://projects.eclipse.org/sites/default/files/mosquitto-200px.png" width="50" height="50" />  
Mosquitto MQTT

### Containerização

<img src="https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcSJl4fp0SkQbTPU5ZxVl6AKWYuKCwM0gIhNtQ&s" width="50" height="50" />  
Docker

---

## 📁 Estrutura do Projeto

A estrutura do projeto está organizada da seguinte maneira:

```
safebot-hub/
├── web-application/      # Código do frontend e backend da aplicação web
│   ├── backend/          # Código do backend (API, controle do robô)
│   └── frontend/         # Código do frontend (interface de controle)
├── arduino-source/       # Código do firmware para o robô (código Arduino)
│   ├── v1/               # Versão 1 do código Arduino
│   └── v2/               # Versão 2 do código Arduino
└── docker-compose.yml    # Arquivo de configuração do Docker para Mosquitto
```

---

## ⚙️ Configuração do Ambiente

### 1️⃣ Instalar o Docker Desktop

Primeiro, instale o **Docker Desktop** em sua máquina. Acesse o site oficial e siga as instruções:

- [Docker Desktop](https://www.docker.com/products/docker-desktop)

---

### 2️⃣ Criar o Container do Mosquitto

Depois de instalar o Docker, abra o terminal e execute o comando abaixo para baixar a imagem do **Mosquitto** (broker MQTT):

```bash
docker pull eclipse-mosquitto
```

Isso irá baixar a imagem do Mosquitto para criar o broker MQTT.

---

### 3️⃣ Instalar a IDE do Arduino

Baixe e instale a **IDE do Arduino** a partir do site oficial:

- [Arduino IDE](https://www.arduino.cc/en/software)

Após a instalação, siga as instruções para configurar o MQTT na placa **VESPA (RoboCore)** acessando o link abaixo:

- [Configuração MQTT RoboCore](https://www.robocore.net/tutoriais/primeiros-passos-com-vespa?srsltid=AfmBOoq22rP2DPoq1rLbcL902274hmmtBQr246GL3fsH7XfL_TSp4nXX)

---

### 4️⃣ Instalar o Node.js

Verifique se o **Node.js** está instalado na sua máquina com o comando:

```bash
node --version
```

Ou, para verificar também:

```bash
node -v
```

Caso o Node.js não esteja instalado, instale a versão **LTS**:

- [Instalar Node.js LTS](https://nodejs.org/)

---

### 5️⃣ Fork e Clone do Repositório

1. **Faça um fork** deste repositório no GitHub.
2. **Clone** o repositório para sua máquina:

```bash
git clone https://github.com/seu-usuario/safebot-hub.git
```

---

### 6️⃣ Instalar Dependências do Projeto

Abra a pasta **`web-application`** na sua IDE de preferência. No terminal, execute:

```bash
npm install
```

Ou:

```bash
npm i
```

Isso irá instalar todas as dependências necessárias para o projeto.

---

### 7️⃣ Configuração de IP

- **No código do Web Application (Backend)**: Abra o arquivo `backend/server.js` e altere o IP para o IP da sua máquina.
- **No código do Firmware (Arduino)**: Abra o código na IDE do Arduino e altere o IP para o da sua máquina.

Para descobrir o IP da sua máquina, use os seguintes comandos:

- **Windows**: `ipconfig`
- **macOS**: `ifconfig`
- **Linux**: `ifconfig -a`

---

### 8️⃣ Configuração da Rede

Certifique-se de que:

- Sua máquina, o robô e o broker MQTT (Mosquitto) estão na **mesma rede Wi-Fi**.
- As portas **1883** (MQTT) e **1880** (outras portas necessárias) estão **abertas**.

_Dica:_ Para testes, use um **hotspot de celular** para evitar bloqueios de rede.

---

### 9️⃣ Compilação do Firmware para a Placa VESPA

Após configurar o IP, **compile e faça o upload** do código do **firmware** para a placa **VESPA** no robô.

---

### 🔟 Iniciando o Projeto

Agora, para iniciar o servidor backend, execute o seguinte comando no terminal da sua IDE:

```bash
node backend/server.js
```

Isso iniciará o projeto e ele estará acessível para dispositivos na mesma rede através do **IP da sua máquina**. Você pode acessar:

- **Porta MQTT**: `1883`
- **Porta Web**: `3000`

---

## 📝 Contribuição

Se você deseja contribuir com o projeto, siga os passos abaixo:

1. Faça um **fork** do repositório.
2. Crie uma **branch** para suas alterações.
3. Submeta um **pull request** com suas contribuições.

---

## 📬 Contato

Caso tenha alguma dúvida ou sugestão, entre em contato através das [**Issues**](https://github.com/HugoRossetti/safebot-hub/issues).
