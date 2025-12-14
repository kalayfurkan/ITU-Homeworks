import argparse
import subprocess
import re
from collections import defaultdict
import itertools
import matplotlib.pyplot as plt


# ----------------- 1) C++ programını çalıştır ----------------- #

def run_cpp(exe, input_file, algo, server=None, budget=None, period=None):
    """
    C++ rts.exe'yi verilen parametrelerle çalıştırır,
    stdout'u string olarak döndürür.

    Çağrı formatları:
      - exe input algo
      - exe input algo background
      - exe input algo deferrable B P   (veya başka server tipleri + B,P)
    """
    cmd = [exe, input_file, algo]

    if server is not None:
        # background server: sadece server adı yeterli
        if server.lower() == "background":
            cmd.append(server)
        else:
            # Diğer server tipleri için budget ve period zorunlu
            if budget is None or period is None:
                raise ValueError(
                    f"Server='{server}' için budget ve period vermen gerekiyor "
                    f"(ör: deferrable 1 3)."
                )
            cmd.extend([server, str(budget), str(period)])

    print("Running command:", " ".join(cmd))  # debug için

    result = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
        check=True
    )
    return result.stdout


# ----------------- 2) Çıktıyı parse et ----------------- #

def parse_schedule(log_text: str):
    """
    C++ programının log çıktısını alır,
    her zaman birimi için hangi job'un çalıştığını çıkarır
    ve her job için (başlangıç_zamanı, süre) aralıkları üretir.

    Desteklenen satır tipleri:
      - At time: 0 TaskT2 is running
      - At time: 1 Server ServerTask serving A1 (budget: 1)   (deferrable vb.)
      - System can run aperiodic task at time:10. + Aperiodic Task: A1 is running. (background)
      - 10 is IDLE
      - System is Idle at 23.
    """

    # Periyodik task: "At time: 0 TaskT2 is running"
    pattern_run = re.compile(
        r'At time:\s*(\d+)\s+(Task\w+)\s+is running',
        re.IGNORECASE
    )

    # Server/aperiyodik (deferrable, polling, vs.):
    # Örn: "At time: 1 Server ServerTask serving A1 (budget: 1)"
    # Budget kısmı opsiyonel:
    pattern_server = re.compile(
        r'At time:\s*(\d+).*Server\s+(\S+).*serving\s+(\S+)(?:\s*\(.*\))?',
        re.IGNORECASE
    )

    # BACKGROUND server:
    # "System can run aperiodic task at time:10."
    pattern_bg_time = re.compile(
        r'System\s+can\s+run\s+aperiodic\s+task\s+at\s+time:(\d+)\.?',
        re.IGNORECASE
    )
    # Hemen arkasından gelen satır:
    # "Aperiodic Task: A1 is running."
    pattern_bg_task = re.compile(
        r'Aperiodic\s+Task:\s*(\w+)\s+is\s+running\.?',
        re.IGNORECASE
    )

    # Eski idle: "10 is IDLE"
    pattern_idle_old = re.compile(
        r'(\d+)\s+is\s+IDLE',
        re.IGNORECASE
    )

    # Yeni idle: "System is Idle at 23."
    pattern_idle_new = re.compile(
        r'System\s+is\s+Idle\s+at\s*(\d+)\.?',
        re.IGNORECASE
    )

    schedule = {}  # time -> label (TaskT1, A1, IDLE vs.)
    pending_bg_time = None  # background için zaman bilgisini tutacağız

    for line in log_text.splitlines():
        line = line.strip()

        # 1) Periyodik task
        m1 = pattern_run.search(line)
        if m1:
            t = int(m1.group(1))
            task = m1.group(2)   # örn: "TaskT2"
            schedule[t] = task
            pending_bg_time = None  # garanti olsun diye temizleyelim
            continue

        # 2) Server / aperiodik job (deferrable / polling vs.)
        m2 = pattern_server.search(line)
        if m2:
            t = int(m2.group(1))
            server_name = m2.group(2)   # "ServerTask" gibi
            aperiodic_job = m2.group(3) # "A1", "A2" vb.

            # Sadece A1/A2 görmek istersen:
            # schedule[t] = aperiodic_job

            schedule[t] = f"{aperiodic_job} (via {server_name})"
            pending_bg_time = None
            continue

        # 3a) Background: önce zaman satırı
        m_bg_t = pattern_bg_time.search(line)
        if m_bg_t:
            pending_bg_time = int(m_bg_t.group(1))
            continue

        # 3b) Background: sonra job satırı
        m_bg_task = pattern_bg_task.search(line)
        if m_bg_task and pending_bg_time is not None:
            job = m_bg_task.group(1)   # A1, A2, ...
            # İster direkt A1 de:
            schedule[pending_bg_time] = job
            # İstersen etiketli:
            # schedule[pending_bg_time] = f"{job} (aperiodic)"
            pending_bg_time = None
            continue

        # 4) Eski idle satırı: "10 is IDLE"
        m3 = pattern_idle_old.search(line)
        if m3:
            t = int(m3.group(1))
            schedule[t] = "IDLE"
            pending_bg_time = None
            continue

        # 5) Yeni idle satırı: "System is Idle at 23."
        m4 = pattern_idle_new.search(line)
        if m4:
            t = int(m4.group(1))
            schedule[t] = "IDLE"
            pending_bg_time = None
            continue

        # Diğer satırlar (header, deadline missed vs.) görmezden geliniyor.

    if not schedule:
        raise ValueError("Log içinde schedule satırı bulunamadı.")

    times = sorted(schedule.keys())

    # Her job için ardışık zaman aralıklarını (start, duration) şeklinde grupla
    task_intervals = defaultdict(list)

    current_task = None
    start_time = None
    prev_time = None

    for t in times:
        task = schedule[t]
        # Job değiştiyse veya zaman kesintiliyse eski bloğu kapat
        if (task != current_task) or (prev_time is None) or (t != prev_time + 1):
            if current_task is not None:
                duration = prev_time - start_time + 1
                task_intervals[current_task].append((start_time, duration))
            current_task = task
            start_time = t
        prev_time = t

    # Son bloğu kapat
    if current_task is not None:
        duration = prev_time - start_time + 1
        task_intervals[current_task].append((start_time, duration))

    max_time = max(times)
    return task_intervals, max_time



# ----------------- 3) Timeline (Gantt) çiz ----------------- #

def plot_timeline(task_intervals, max_time, title="Schedule Timeline"):
    """
    task_intervals: { 'TaskT1': [(start, duration), ...], ... }
    max_time: en büyük zaman birimi (x-axis için)
    """
    fig, ax = plt.subplots(figsize=(10, 3))

    # Task isimlerini sırala, IDLE en alta gelsin
    task_names = sorted(task_intervals.keys())
    if "IDLE" in task_names:
        task_names.remove("IDLE")
        task_names.append("IDLE")

    y_positions = range(len(task_names))

    # Matplotlib default color cycle
    colors = plt.rcParams['axes.prop_cycle'].by_key()['color']
    color_cycle = itertools.cycle(colors)

    for y, task in zip(y_positions, task_names):
        intervals = task_intervals[task]  # [(start, duration), ...]
        ax.broken_barh(
            intervals,
            (y - 0.4, 0.8),
            facecolors=next(color_cycle),
            label=task
        )

    ax.set_yticks(list(y_positions))
    ax.set_yticklabels(task_names)
    ax.set_xlabel("Time")
    ax.set_xlim(0, max_time + 1)

    ax.grid(True, axis="x", linestyle="--", alpha=0.5)
    ax.set_title(title)

    # Legend’de tekrarları temizle
    handles, labels = ax.get_legend_handles_labels()
    uniq = dict(zip(labels, handles))
    ax.legend(uniq.values(), uniq.keys(), loc="upper right")

    plt.tight_layout()
    plt.show()


# ----------------- 4) Argümanları al, her şeyi bağla ----------------- #

def main():
    parser = argparse.ArgumentParser(
        description="RTS C++ scheduler çıktısını timeline olarak çiz."
    )

    # Pozisyonel argümanlar: exe, input, algo zorunlu
    parser.add_argument("exe", help="Çalıştırılacak exe (ör: rts.exe veya ./rts)")
    parser.add_argument("input_file", help="Input dosyası (ör: inputs.txt)")
    parser.add_argument("algorithm", help="Algoritma adı (ör: rm, dm, edf, ...)")
    # Bunlar opsiyonel: server, budget, period
    parser.add_argument("server", nargs="?", default=None, help="Server adı (opsiyonel)")
    parser.add_argument("budget", nargs="?", default=None, help="Server budget (opsiyonel)")
    parser.add_argument("period", nargs="?", default=None, help="Server period (opsiyonel)")

    args = parser.parse_args()

    try:
        log_text = run_cpp(
            args.exe,
            args.input_file,
            args.algorithm,
            server=args.server,
            budget=args.budget,
            period=args.period
        )

        task_intervals, max_time = parse_schedule(log_text)

        title = f"{args.algorithm.upper()} Schedule Timeline"
        if args.server:
            if args.server.lower() == "background":
                title += f" (Server: {args.server})"
            elif args.budget and args.period:
                title += f" (Server: {args.server}, B={args.budget}, P={args.period})"
            else:
                # güvenli fallback
                title += f" (Server: {args.server})"

        # ÖNEMLİ: Grafiği gerçekten çiz
        plot_timeline(task_intervals, max_time, title=title)

    except subprocess.CalledProcessError as e:
        print("C++ programını çalıştırırken hata oluştu!")
        print("Komut:", " ".join(e.cmd))
        print("Return code:", e.returncode)
        print("Stdout:\n", e.stdout)
        print("Stderr:\n", e.stderr)
    except Exception as e:
        print("Hata:", e)


if __name__ == "__main__":
    main()
