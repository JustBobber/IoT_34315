# Kom i gang med projektet

## Forudsætninger

- Python 3.3 eller nyere (tjek med `python --version` eller `python3 --version`)

---

## Opsætning

### 1. Klon projektet

```bash
git clone https://github.com/JustBobber/IoT_34315
cd IoT_34315
```

### 2. Opret virtuelt miljø

```bash
python3 -m venv .venv
```

### 3. Aktivér miljøet

**macOS / Linux:**
```bash
source .venv/bin/activate
```

**Windows:**
```bash
.venv\Scripts\activate
```

Du kan se at miljøet er aktivt når terminalen viser `(.venv)` foran din prompt.

### 4. Installér afhængigheder

```bash
pip install -r requirements.txt
```

---

# Daglig brug
For at køre projektet er der to ting du skal have kørende: webappen og ESP'en.
Webappen står for backed og frontend. Der modtager data fra ESP, logger det i database og laver en hjemmeside hvor dataen kan ses.

ESP'en står indsamler og sender data til webappen når en træning session er i gang. Ellers poller den serveren for at 
tjekke om der er en user logget ind da dette er et krav for at kunne påbegynde en træningssession. 

## Start server, webapp og populer database

### Aktiver virtual enviroment
Hver gang du åbner en ny terminal skal du aktivere miljøet igen:

```bash
source .venv/bin/activate   # macOS / Linux
.venv\Scripts\activate      # Windows
```

### Start webappen
```bash
python3 src/app.py
```
besøg siden: localhost:5050


Deaktivér venv når du er færdig:

```bash
deactivate
```

---

### Populer databasen: 
For at få noget data i databasen kør følgende script:

```bash
python3 src/seeds/alice_and_bob_seed.py
```
Det tilføjer et par users og generere et par sessions og noget data. 
Det kan køres flere gange hvis der ønskes mere data. 

### Slet databasen:
Det kan være nødvendigt/rart at slette database (training.db) den ligger her: src/training.db
```bash
rm src/training.db
```
Kør derefter seed scriptet igen for at få data i databasen.

---

## Start ESP
Beskriv hvad der skal til får at køre ESP'erne.

### Wiring se diagram ... 
<indsæt billede af wiring diagram>

### Download firmware
Download /firmware/.. til esp'erne ...

