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
python -m venv .venv
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

## aktiver virtual enviroment
Hver gang du åbner en ny terminal skal du aktivere miljøet igen:

```bash
source .venv/bin/activate   # macOS / Linux
.venv\Scripts\activate      # Windows
```

## start webappen
```bash
python3 src/app.py
```
besøg siden: localhost:5050


Deaktivér venv når du er færdig:

```bash
deactivate
```

---

## populer databasen: 
For at få noget data i databasen kør følgede script:

```bash
python3 src/seeds/alice_and_bob_seed.py
```
Det tilføjer et par users og generere et par sessions og noget data. 
Det kan køres flere gange hvis der ønskes mere data. 

## slet databasen:
Det kan være nødvendigt/rart at slette database (training.db) den bør ligge her: src/training.db
