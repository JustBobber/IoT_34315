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

## Tilføj nye pakker

```bash
pip install <pakkenavn>
pip freeze > requirements.txt   # Opdatér requirements.txt bagefter
```
