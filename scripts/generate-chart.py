import matplotlib
matplotlib.use("Agg")  # headless: render to file, no display needed
import matplotlib.pyplot as plt
from datetime import datetime
import os

dates = [
  datetime(2022, 3, 27),
  datetime(2024, 1, 1),
  datetime(2024, 7, 19),
  datetime(2024, 11, 1),
  datetime(2024, 11, 5),
  datetime(2024, 11, 10),
  datetime(2025, 2, 5),
  datetime(2025, 12, 28),  # was Dec 27, actual commit is Dec 28
  datetime(2026, 5, 23),
  datetime(2026, 5, 24),
  datetime(2026, 7, 1),
  datetime(2026, 7, 6),
]

decompiled_code = [
  0.0,
  0.0,   # no asm structure at this commit (was manually estimated 2.0)
  8.6,
  8.6,
  10.4,
  10.6,
  11.0,
  14.3,
  15.2,
  18.0,
  18.9,
  30.3,
]
matched_code = [
  0.0,
  0.0,   # same
  1.0,
  1.0,
  1.0,
  1.0,
  1.0,
  1.0,
  1.0,
  1.0,
  1.0,
  1.0,
]


fig, ax = plt.subplots()

ax.plot(dates, decompiled_code, color='#4659eb', marker='.', markersize=8, linewidth=2.5)
# ax.plot(dates, matched_code, label='Matched Code', color='#5a6ae6', marker='.', markersize=8, linestyle='--', linewidth=1.5)

ax.set_title("Decompiled Code Over Time")
ax.set_xlabel("Date")
ax.set_ylabel("Percentage (%)")
ax.legend(loc='upper left')
plt.xticks(rotation=45)
plt.tight_layout()
plt.ylim([0, 100])
plt.grid()


out_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(out_dir, "progress.png")
plt.savefig(path)
print(f"Wrote {path}")
