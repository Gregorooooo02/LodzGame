# ?? Water Slowdown Component - Instrukcja

## ?? Co robi ten komponent:

**WaterSlowdownComponent** automatycznie spowalnia gracza w zale¿noœci od poziomu wody:

- **Nie w wodzie** (0% zanurzenia) ? Prêdkoœæ 100%
- **Czêœciowo w wodzie** (50% zanurzenia) ? Prêdkoœæ ~75%
- **Ca³kowicie zanurzony** (100% zanurzenia) ? Prêdkoœæ 50%

**Dotyczy:**
- ? Prêdkoœæ chodzenia (Walk Speed)
- ? Wysokoœæ skoku (Jump Velocity)
- ? Grawitacja (Gravity Scale)

---

## ?? Setup (Krok po kroku):

### 1. W Blueprint postaci gracza (np. BP_ThirdPersonCharacter):

1. **Otwórz Blueprint** postaci gracza
2. **Components** panel ? **Add Component**
3. Szukaj: **"Water Slowdown Component"**
4. Dodaj do postaci

### 2. Skonfiguruj komponent:

W **Details** panel, sekcja **"Water Slowdown"**:

| Parametr | Domyœlnie | Opis |
|----------|-----------|------|
| `Min Speed Multiplier` | 0.5 | Prêdkoœæ przy pe³nym zanurzeniu (0.5 = 50%) |
| `Min Jump Multiplier` | 0.5 | Wysokoœæ skoku przy pe³nym zanurzeniu |
| `Min Gravity Multiplier` | 0.5 | Grawitacja przy pe³nym zanurzeniu |
| `Water Manager` | None | **Przeci¹gnij BP_WaterLevelManager!** |
| `Character Height` | 180.0 | Wysokoœæ postaci (cm) |
| `Transition Speed` | 5.0 | Jak szybko nastêpuje zmiana (wy¿sze = szybciej) |

**?? WA¯NE:** Musisz przypisaæ `Water Manager`! Bez tego komponent nie zadzia³a!

---

## ?? Jak to dzia³a:

### Obliczanie poziomu zanurzenia:

```
Pozycja gracza: Z = 100
Wysokoœæ gracza: Height = 180
Poziom wody: WaterLevel = 150

G³êbokoœæ w wodzie = WaterLevel - (CharacterZ - Height/2)
                    = 150 - (100 - 90)
                    = 140 cm w wodzie

Poziom zanurzenia = 140 / 180 = 77.7%
```

### Przyk³adowe wartoœci:

| Zanurzenie | Prêdkoœæ | Skok | Grawitacja |
|------------|----------|------|------------|
| 0% (suchy) | 600 | 420 | 1.0 |
| 25% | 537.5 | 367.5 | 0.875 |
| 50% | 550 | 367.5 | 0.75 |
| 75% | 412.5 | 315 | 0.625 |
| 100% (zanurzony) | 300 | 210 | 0.5 |

---

## ?? Zaawansowane ustawienia:

### Chcesz wolniejsze spowalnianie?
```
Min Speed Multiplier = 0.3 (30% prêdkoœci)
```

### Chcesz ¿eby skok by³ bardziej ograniczony?
```
Min Jump Multiplier = 0.2 (20% wysokoœci skoku)
```

### Chcesz natychmiastow¹ zmianê (bez p³ynnego przejœcia)?
```
Transition Speed = 100.0
```

### Chcesz wolne przejœcie?
```
Transition Speed = 1.0
```

---

## ?? Dodatkowe pomys³y:

### 1. Ró¿ne efekty dla ró¿nych postaci:

W Blueprincie gracza mo¿esz zmieniæ wartoœci dynamicznie:

```
Event BeginPlay
  ?
GET WaterSlowdownComponent
  ?
SET Min Speed Multiplier = 0.3 (dla wolniejszej postaci)
```

### 2. Efekty dŸwiêkowe:

```
Event Tick
  ?
GET WaterSlowdownComponent ? GET CurrentSpeedMultiplier
  ?
Branch (SpeedMultiplier < 0.9?)
  TRUE ? Play Sound "Splash" (if not playing)
```

### 3. Efekty wizualne:

```
If (Submersion > 0.5):
  ?? Spawn Particle: Water Ripples
  ?? Post Process: Underwater blur
```

---

## ?? Troubleshooting:

**Problem:** Gracz siê nie spowalnia
- ? SprawdŸ czy `Water Manager` jest przypisany
- ? SprawdŸ czy `BP_WaterLevelManager` jest w levellu
- ? SprawdŸ czy `WaterLevel` siê zmienia (dodaj Print String)

**Problem:** Spowalnia siê za szybko/wolno
- ?? Zmieñ `Transition Speed`

**Problem:** Spowalnia siê za bardzo
- ?? Zmieñ `Min Speed Multiplier` (np. 0.7 zamiast 0.5)

**Problem:** Nie dzia³a skok w wodzie
- ? Upewnij siê ¿e postaæ ma `CharacterMovementComponent`

---

## ?? Quick Setup Checklist:

- [ ] Doda³em `WaterSlowdownComponent` do postaci gracza
- [ ] Przypisa³em `Water Manager` (BP_WaterLevelManager z levelu)
- [ ] Ustawi³em `Character Height` (domyœlnie 180.0)
- [ ] Przetestowa³em w grze - gracz spowalnia w wodzie ?

---

**Gotowe! Gracz bêdzie siê teraz spowalnia³ w wodzie!** ???????

## ?? Przyk³adowe wartoœci dla ró¿nych gier:

**Realistic (horror):**
```
Min Speed Multiplier = 0.3
Min Jump Multiplier = 0.2
Min Gravity Multiplier = 0.4
Transition Speed = 3.0
```

**Arcade (fun):**
```
Min Speed Multiplier = 0.7
Min Jump Multiplier = 0.8
Min Gravity Multiplier = 0.6
Transition Speed = 10.0
```

**Platformer:**
```
Min Speed Multiplier = 0.6
Min Jump Multiplier = 0.5
Min Gravity Multiplier = 0.3 (powolne opadanie!)
Transition Speed = 5.0
```
