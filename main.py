import time
import sys

# Since you're a TUI fan, let's add some color constants
G = "\033[32m" # Green
B = "\033[34m" # Blue
R = "\033[31m" # Red
W = "\033[0m"  # Reset

def slow_print(text, delay=0.03):
    for char in text:
        sys.stdout.write(char)
        sys.stdout.flush()
        time.sleep(delay)
    print()

def main():
    # Room Data Structure
    world = {
        "start": {
            "desc": "You awaken in a damp cave. A single torch flickers.",
            "paths": {"north": "waterfall", "east": "glade"}
        },
        "waterfall": {
            "desc": "A frozen waterfall shimmers here. There's a weird ice formation.",
            "paths": {"examine": "portal", "back": "start"}
        },
        "glade": {
            "desc": "A sunlit glade. A friendly robin chirps at you.",
            "paths": {"approach": "robin", "back": "start"}
        },
        "portal": {
            "desc": f"{B}The ice forms a portal!{W} Do you step through?",
            "paths": {"step": "game_over", "stay": "waterfall"}
        },
        "robin": {
            "desc": "The robin offers a berry. It looks... suspiciously delicious.",
            "paths": {"accept": "win", "decline": "glade"}
        }
    }

    current_room = "start"

    slow_print(f"{G}--- ADVENTURE OS v0.1 ---{W}\n")

    while True:
        room = world[current_room]
        slow_print(f"\n{room['desc']}")
        
        # Show available commands
        options = ", ".join(room['paths'].keys())
        choice = input(f"({options}) > ").lower().strip()

        if choice in room['paths']:
            current_room = room['paths'][choice]
            
            if current_room == "game_over":
                slow_print(f"{R}You stepped through and got lost in the void. Game Over!{W}")
                break
            elif current_room == "win":
                slow_print(f"{G}The berry gives you super powers. You win!{W}")
                break
        else:
            print(f"[{R}!{W}] That action isn't mapped to this sector.")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nExiting...")