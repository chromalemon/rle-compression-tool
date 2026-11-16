from pathlib import Path
import os
import random

out = Path("benchmark/data")
out.mkdir(parents=True, exist_ok=True)

# 1. Repeated data: 10 MB of identical bytes.
(out / "repeated.bin").write_bytes(b"A" * 10_000_000)

# 2. Random data: 10 MB of pseudorandom bytes.
(out / "random.bin").write_bytes(os.urandom(10_000_000))

# 3. Mixed data: 10 MB consisting of alternating repetitive
#    and random regions, each 100 KB long.
rng = random.Random(42)
parts = []

for i in range(100):
    if i % 2 == 0:
        parts.append(b"A" * 100_000)
    else:
        parts.append(bytes(rng.getrandbits(8) for _ in range(100_000)))

(out / "mixed.bin").write_bytes(b"".join(parts))

# 4. Repetitive text: a realistic, repeated text record.
record = (
    "Event: Imperial Computing Society Meetup\n"
    "Location: London\n"
    "Category: Technology\n"
    "Description: Join us for talks, networking and workshops.\n"
    "Attendees: 125\n\n"
)

text = (record * (1_000_000 // len(record) + 1))[:1_000_000]
(out / "text.txt").write_text(text, encoding="utf-8")

print("Generated benchmark files:")
for path in sorted(out.iterdir()):
    print(f"{path}: {path.stat().st_size:,} bytes")
