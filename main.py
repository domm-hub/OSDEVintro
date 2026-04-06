import time
import random

# TUI Styling
G, B, R, Y, W = "\033[32m", "\033[34m", "\033[31m", "\033[33m", "\033[0m"

def slow_print(text, delay=0.02):
    for char in text:
        import sys
        sys.stdout.write(char)
        sys.stdout.flush()
        time.sleep(delay)
    print()

class Player:
    def __init__(self):
        self.hp = 100
        self.backpack = ["rusty_knife"]
        self.location = "cave"
        self.is_alive = True

class Monster:
    def __init__(self, name, hp, damage):
        self.name = name
        self.hp = hp
        self.damage = damage

def combat(player, monster):
    slow_print(f"\n{R}!!! A WILD {monster.name.upper()} APPEARS !!!{W}")
    
    while monster.hp > 0 and player.hp > 0:
        print(f"\nYour HP: {G}{player.hp}{W} | {monster.name} HP: {R}{monster.hp}{W}")
        action = input("Actions: (stab / heal / run) > ").lower()

        if action == "stab":
            dmg = random.randint(15, 25)
            monster.hp -= dmg
            slow_print(f"You slash the {monster.name} for {dmg} damage!")
        elif action == "heal":
            if "berry" in player.backpack:
                player.hp += 30
                player.backpack.remove("berry")
                slow_print(f"{G}You ate a berry! +30 HP{W}")
            else:
                print("No berries in your backpack!")
                continue
        elif action == "run":
            slow_print("You tried to run, but it blocked the exit!")
        
        if monster.hp > 0:
            p_dmg = random.randint(10, 20)
            player.hp -= p_dmg
            slow_print(f"The {monster.name} bites you for {p_dmg} damage!")

    if player.hp <= 0:
        player.is_alive = False
        slow_print(f"{R}You died in the dark... Game Over.{W}")
    else:
        slow_print(f"{G}The {monster.name} melts into a puddle of XP. You win!{W}")

def game():
    p = Player()
    goblin = Monster("Cave Goblin", 60, 15)
    
    world = {
        "cave": {"desc": "Damp cave. Paths go North, East, and South.", "exits": {"n": "waterfall", "e": "glade", "s": "boss_room"}},
        "glade": {"desc": "Sunlit glade. A robin chirps.", "exits": {"w": "cave"}},
        "waterfall": {"desc": "A wall of ice.", "exits": {"s": "cave"}}
    }

    while p.is_alive:
        room = world[p.location]
        slow_print(f"\n[{p.location.upper()}] - {room['desc']}")
        
        cmd = input(f"{B}Command{W} (n/e/s/w/look/inv) > ").lower()

        if cmd in room["exits"]:
            p.location = room["exits"][cmd]
            
            # TRIGGER COMBAT IN SOUTH
            if p.location == "boss_room":
                combat(p, goblin)
                if p.is_alive: p.location = "cave" # Return after win
        
        elif cmd == "look" and p.location == "glade":
            if "berry" not in p.backpack:
                slow_print("You found a magic berry! Added to backpack.")
                p.backpack.append("berry")
        
        elif cmd == "inv":
            print(f"Backpack: {Y}{', '.join(p.backpack)}{W}")

if __name__ == "__main__":
    game()