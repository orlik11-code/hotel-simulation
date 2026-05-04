//
// Created by orlik on 27.04.2026.
//

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "Apartment.h"
#include "DoubleRoom.h"
#include "Hotel.h"
#include "SingleRoom.h"
#include "TripleRoom.h"

void printMenu() {
  std::cout << "\n--- SYSTEM ZARZADZANIA HOTELEM ---\n";
  std::cout << "1. Dodaj Pokoj Jednoosobowy\n";
  std::cout << "2. Dodaj Pokoj Dwuosobowy\n";
  std::cout << "3. Dodaj Pokoj Trzyosobowy\n";
  std::cout << "4. Dodaj Apartament\n";
  std::cout << "5. Usun pokoj (po numerze)\n";
  std::cout << "6. Wyswietl raport\n";
  std::cout << "0. Wyjscie\n";
  std::cout << "Wybor: ";
}

int main() {
  Hotel hotel;
  int choice;
  unsigned int nr, floor, s_beds, d_beds;
  double area;

  do {
    printMenu();
    if (!(std::cin >> choice)) {
      std::cin.clear();
      std::cin.ignore(1000, '\n');
      continue;
    }

    try {
      if (choice >= 1 && choice <= 4) {
        std::cout << "Podaj numer pokoju: ";
        std::cin >> nr;
        std::cout << "Podaj pietro: ";
        std::cin >> floor;
        std::cout << "Podaj metraz: ";
        std::cin >> area;
      }

      switch (choice) {
      case 1:
        hotel.addRoom(
            std::make_unique<SingleRoom>(area, nr, floor, 1, 0, false, false));
        std::cout << "Dodano SingleRoom.\n";
        break;
      case 2:
        std::cout << "Liczba lozek pojedynczych (0-2): ";
        std::cin >> s_beds;
        std::cout << "Liczba lozek podwojnych (0-1): ";
        std::cin >> d_beds;
        hotel.addRoom(std::make_unique<DoubleRoom>(area, nr, floor, s_beds,
                                                   d_beds, false, false));
        std::cout << "Dodano DoubleRoom.\n";
        break;
      case 3:
        std::cout << "Liczba lozek pojedynczych: ";
        std::cin >> s_beds;
        std::cout << "Liczba lozek podwojnych: ";
        std::cin >> d_beds;
        hotel.addRoom(std::make_unique<TripleRoom>(area, nr, floor, s_beds,
                                                   d_beds, false, false));
        std::cout << "Dodano TripleRoom.\n";
        break;
      case 4:
        std::cout << "Liczba lozek pojedynczych: ";
        std::cin >> s_beds;
        std::cout << "Liczba lozek podwojnych: ";
        std::cin >> d_beds;
        hotel.addRoom(std::make_unique<Apartment>(area, nr, floor, s_beds,
                                                  d_beds, false, false));
        std::cout << "Dodano Apartment.\n";
        break;
      case 5:
        std::cout << "Podaj numer pokoju do usuniecia: ";
        std::cin >> nr;
        hotel.removeRoom(nr);
        std::cout << "Wykonano probe usuniecia pokoju nr " << nr << ".\n";
        break;
      case 6:
        hotel.showReport();
        break;
      case 0:
        std::cout << "Koniec programu.\n";
        break;
      default:
        std::cout << "Bledny wybor!\n";
      }
    } catch (const std::exception &e) {
      std::cerr << "BLAD: " << e.what() << "\n";
    }

  } while (choice != 0);

  return 0;
}