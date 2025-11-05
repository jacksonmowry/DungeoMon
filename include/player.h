

typedef struct Player {
    int health;
    int armor;
    int mana;
    int dmg;
    void* spells;  // NOTE: Support Later
    void* weapons; // NOTE: Support Later
    int num_spells;
    int num_weapons;
} Player;
