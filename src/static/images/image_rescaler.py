from PIL import Image
from pathlib import Path

files = {
    "Calender.png": 64,
    "Duration.png": 64,
    "Streak.png": 64,
    "Progression.png": 64,
    "Genstraek_logo.png": 140
}

base_path = Path(__file__).parent

for filename, size in files.items():
    input_path = base_path / filename
    output_path = base_path / f"small_{filename}"

    img = Image.open(input_path).convert("RGBA")
    img.thumbnail((size, size), Image.LANCZOS)
    img.save(output_path, optimize=True)

    print(f"Saved: {output_path}")