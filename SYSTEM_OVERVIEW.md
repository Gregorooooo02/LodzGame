# ?? LodzGame - Complete Water & Valve System

## ?? Zaimplementowane Systemy:

### 1. ?? **Valve System (Zawór)**
- Fizyczne obracanie zaworu myszk¹
- Automatyczne odpadniêcie po 720°
- W³¹czenie fizyki na odpadniêtym kole
- Pe³na integracja z systemem wody

**Pliki:**
- `VValveAtor.h/cpp` - G³ówna logika zaworu
- `ValveInteractionComponent.h/cpp` - Komponent dla gracza
- `VALVE_SETUP_README.md` - Pe³na instrukcja

---

### 2. ?? **Water Level Manager (Manager Poziomu Wody)**
- Automatyczne podnoszenie poziomu wody
- P³ynne opadanie po aktywacji zaworu
- Konfigurowalne prêdkoœci i limity
- Automatyczne pozycjonowanie wody

**Pliki:**
- `WaterLevelManager.h/cpp` - C++ implementacja
- `WATER_MANAGER_INTEGRATION.md` - Dokumentacja integracji

**Funkcje:**
- `LowerWater(Amount)` - Obni¿ wodê o wartoœæ
- `StartLowering(Amount)` - Rozpocznij opadanie
- `StopLowering()` - Zatrzymaj opadanie

---

### 3. ?? **Water Slowdown Component (Spowalnianie w Wodzie)**
- Automatyczne spowalnianie gracza w wodzie
- Wp³yw na prêdkoœæ, skok i grawitacjê
- P³ynne przejœcia miêdzy stanami
- Obliczanie poziomu zanurzenia

**Pliki:**
- `WaterSlowdownComponent.h/cpp` - C++ komponent
- `WATER_SLOWDOWN_SETUP.md` - Instrukcja u¿ycia

---

## ?? Quick Start:

### Krok 1: Setup Wody
```
1. Umieœæ BP_WaterLevelManager w levellu
2. Ustaw Water Body (Plane z materia³em wody)
3. Skonfiguruj prêdkoœci (Rising/Lowering Speed)
```

### Krok 2: Setup Zaworu
```
1. Umieœæ BP_Valve w levellu
2. Przypisz mesh do ValveMesh
3. Przypisz BP_WaterLevelManager do Water Level Manager
4. Dodaj ValveInteractionComponent do gracza
5. Zbinduj klawisz E do TryInteractWithValve
```

### Krok 3: Setup Spowalniania
```
1. Dodaj WaterSlowdownComponent do postaci gracza
2. Przypisz BP_WaterLevelManager do Water Manager
3. Ustaw Character Height (180.0)
4. Dostosuj Min Speed/Jump/Gravity Multiplier
```

---

## ?? Parametry i Wartoœci:

### WaterLevelManager:
| Parametr | Domyœlnie | Opis |
|----------|-----------|------|
| Water Level | 0.0 | Aktualny poziom (Z position) |
| Max Water Level | 100.0 | Górny limit |
| Min Water Level | -100.0 | Dolny limit |
| Rising Speed | 10.0 | Prêdkoœæ podnoszenia |
| Lowering Speed | 50.0 | Prêdkoœæ opadania |

### VValveAtor:
| Parametr | Domyœlnie | Opis |
|----------|-----------|------|
| Mouse Sensitivity | 2.0 | Czu³oœæ myszy |
| Required Rotation | 720.0 | Stopnie do przekrêcenia |
| Interaction Distance | 300.0 | Max odleg³oœæ |
| Water Level Decrease | 100.0 | O ile obni¿a wodê |

### WaterSlowdownComponent:
| Parametr | Domyœlnie | Opis |
|----------|-----------|------|
| Min Speed Multiplier | 0.5 | Prêdkoœæ przy zanurzeniu |
| Min Jump Multiplier | 0.5 | Skok przy zanurzeniu |
| Min Gravity Multiplier | 0.5 | Grawitacja przy zanurzeniu |
| Character Height | 180.0 | Wysokoœæ postaci |
| Transition Speed | 5.0 | Prêdkoœæ przejœcia |

---

## ?? Przyk³adowy Flow Gry:

```
1. Gra siê rozpoczyna
   ?? Woda: poziom 0, roœnie do 100

2. Gracz podchodzi do zaworu
   ?? Prompt: "Press E to use valve"

3. Gracz naciska E
   ?? Kamera siê blokuje
   ?? Mysz kontroluje zawór

4. Gracz krêci myszk¹
   ?? Zawór siê obraca
   ?? Licznik: 0°/720°

5. Po 720°
   ?? Zawór odpada (fizyka)
   ?? Kamera siê odblokowuje
   ?? LowerWater(100) wywo³ane

6. Woda opada
   ?? Z 100 do 0 (szybko, LoweringSpeed=50)
   ?? Gracz mo¿e przejœæ dalej

7. Po osi¹gniêciu celu
   ?? Woda znowu zaczyna rosn¹æ do 100
   ?? Cykl siê powtarza
```

---

## ?? Dodatkowe Funkcje:

### Event System (TODO):
- `OnWaterReachedMax` - Gdy woda osi¹gnie maksimum
- `OnWaterReachedMin` - Gdy woda osi¹gnie minimum
- `OnValveDetached` - Gdy zawór odpadnie

### Sound System (TODO):
- DŸwiêk krêcenia zaworu
- DŸwiêk opadaj¹cej wody
- Plusk przy wchodzeniu do wody

### VFX System (TODO):
- Particle effects przy odpadniêciu zaworu
- Fale na wodzie
- Underwater post-process

---

## ?? Struktura Projektu:

```
Source/LodzGame/
??? Public/
?   ??? VValveAtor.h
?   ??? ValveInteractionComponent.h
?   ??? WaterLevelManager.h
?   ??? WaterSlowdownComponent.h
??? Private/
?   ??? VValveAtor.cpp
?   ??? ValveInteractionComponent.cpp
?   ??? WaterLevelManager.cpp
?   ??? WaterSlowdownComponent.cpp

Dokumentacja/
??? VALVE_SETUP_README.md
??? WATER_MANAGER_INTEGRATION.md
??? WATER_SLOWDOWN_SETUP.md
??? SYSTEM_OVERVIEW.md (ten plik)
```

---

## ?? Common Issues:

**Zawór siê nie obraca:**
- SprawdŸ mesh collision
- SprawdŸ czy ValveMesh jest przypisany

**Woda nie opada:**
- SprawdŸ czy WaterLevelManager jest przypisany
- SprawdŸ logi: "Called LowerWater Blueprint function"

**Gracz siê nie spowalnia:**
- SprawdŸ czy WaterSlowdownComponent ma przypisany WaterManager
- SprawdŸ Character Height

**Kamera siê nie odblokowuje:**
- SprawdŸ logi: "Valve interaction stopped"
- Problem powinien byæ naprawiony w obecnej wersji

---

## ? Checklist Przed Testem:

- [ ] BP_WaterLevelManager w levellu z przypisanym Water Body
- [ ] BP_Valve w levellu z przypisanym mesh?? i WaterLevelManager
- [ ] ValveInteractionComponent dodany do gracza
- [ ] Input zbindowany (klawisz E)
- [ ] WaterSlowdownComponent dodany do gracza z WaterManager
- [ ] Projekt siê kompiluje bez b³êdów
- [ ] Wszystkie logi s¹ widoczne (Window ? Output Log)

---

**System jest kompletny i gotowy do u¿ycia!** ??

Autorzy: AI Assistant & User
Wersja: 1.0
Data: 2024
