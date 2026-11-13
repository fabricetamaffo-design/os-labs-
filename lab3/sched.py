
#!/usr/bin/env python3
import sys

def simulate(policy, lengths, quantum=1):
    jobs = [
        {"name": chr(65 + i), "left": n, "first": None, "done": None}
        for i, n in enumerate(lengths)
    ]

    order = sorted(jobs, key=lambda j: j["left"]) if policy == "SJF" else list(jobs)
    t, timeline = 0, []

    if policy in ("FIFO", "SJF"):
        for j in order:
            j["first"] = t
            timeline.append((t, j["name"], j["left"]))
            t += j["left"]
            j["left"] = 0
            j["done"] = t

    elif policy == "RR":
        queue = list(order)
        while queue:
            j = queue.pop(0)
            if j["first"] is None:
                j["first"] = t

            run = min(quantum, j["left"])
            timeline.append((t, j["name"], run))
            t += run
            j["left"] -= run

            if j["left"] > 0:
                queue.append(j)
            else:
                j["done"] = t
    else:
        sys.exit("Unknown scheduling policy")

    print(f"\nPolicy: {policy}")
    print("Job  Response  Turnaround")
    for j in jobs:
        print(f"{j['name']:>3} {j['first']:>9} {j['done']:>11}")

    avg_response = sum(j["first"] for j in jobs) / len(jobs)
    avg_turnaround = sum(j["done"] for j in jobs) / len(jobs)

    print(f"Average response: {avg_response:.2f}")
    print(f"Average turnaround: {avg_turnaround:.2f}")

if __name__ == "__main__":
    policy = sys.argv[1].upper()
    lengths = [int(x) for x in sys.argv[2].split(",")]
    quantum = int(sys.argv[3]) if len(sys.argv) > 3 else 1
    simulate(policy, lengths, quantum)
