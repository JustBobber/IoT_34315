from datetime import datetime

def calculate_duration(start_time, end_time):
    """
    Hjælpe funktion der:
    Tager et start og slut tidspunkt og beregner tiden i mellem de to.
    :param start_time: Sql timestamp
    :param end_time: Sql timestamp
    :return: formateret string med duration mellem de to tider i minutter og sekunder
    """
    if not (start_time and end_time):
        return "0m 0s"
    start = datetime.strptime(start_time, "%Y-%m-%d %H:%M:%S")
    end = datetime.strptime(end_time, "%Y-%m-%d %H:%M:%S")
    delta = end - start
    minutes = delta.seconds // 60
    seconds = delta.seconds % 60

    return f"{minutes}m {seconds}s"

def get_duration_sum(sessions):
    """
    Hjælpe funktion der:
    Tager en liste af sessions og beregner den totale varighed af sessionerne ved at summere varigheden af hver session.
    :param sessions: Liste af objekter der indeholder obj["start_time"] og obj["end_time"]
    :return: Formateret string der angiver sessioners totale varighed i formatet "<timer> t <minutter> m <sekunder> s"
    """
    total_seconds = 0
    if not sessions:
        return None

    for s in sessions:
        if s["start_time"] and s["end_time"]:
            start = datetime.strptime(s["start_time"], "%Y-%m-%d %H:%M:%S")
            end = datetime.strptime(s["end_time"], "%Y-%m-%d %H:%M:%S")
            total_seconds += (end - start).total_seconds()

    hours = int(total_seconds // 3600)
    minutes = int((total_seconds % 3600) // 60)
    seconds = int(total_seconds % 60)
    return f"{hours}t {minutes}m {seconds}s"
