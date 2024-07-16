# SysOpy-2023-2024
Repository for SysOpy (Systemy Operacyjne, Operating Systems) course at AGH University

W katalogu **laby** znajdują się podkatalogi **lab*i*, i=1,2,...,12**, a każdy z tych podkatalogów zawiera plik *zadanie.pdf* (jedynie w lab12 jest `.txt`) z treścią zadania / zadań na dane laboratorium oraz folder / foldery z rozwiązaniami tych zadań, które zawierają pliki źródłowe `.c`, `.h` oraz Makefile, a czasem także pliki `.txt` z odpowiedziami na pytania w poleceniu.

Z kolei w katalogu **kolosy** znajdują się podkatalogi **kol1** oraz **kol2** odpowiadające poszczególnym kolokwiom. W każdym z tych podkatalogów znajdują się 3 foldery:
- *Tasks_template*, zawierający oryginalne polecenia do zadań z kolokwium
- *Tasks*, zawierający moje rozwiązania kolokwium, wysłane na Upel
- *probne*, który zawiera kolokwia, na których ćwiczyłem do właściwego kolosa

Z kolokwium 1. dostałem 4.0 - w zadaniu 2 nie dodałem flagi O_CREAT oraz w zadaniu 3. zamiast 
`char * const args[] = { argv[1], argv[2], argv[3], NULL };
execvp("./calc", args);`
powinno być
`char* const calcParams[]={"./calc",argv[1],argv[2],argv[3],NULL};
execv("./calc", calcParams);` 
Na kolokwium 2. nie miałem żadnych błędów (albo przynajmniej nie zauważył ich sprawdzajacy).

Moje rozwiązania zadań starałem się trzymać w prostej postaci, więc pewnie czasem można było je zrobić lepiej, ładniej - chociażby rozdzielając program na więcej plików, kiedy było dużo kodu, także moje Makefile są najprostsze możliwe - ot, żeby `make all` zbudowało to co trzeba i lecimy.
Mimo wszystko powinny być w miarę poprawne, jedynie w labach 11. i 12. (z socketów) opcja "Keep Alive" miała dosłownie pingować a nie to dziwactwo co ja tam zrobiłem, ale mimo wszystko rozwiązania tych (i pozostałych) zadań dostały punkt.

Mam nadzieję, że zamieszczone tu materiały okażą się przydatne. Korzystajcie rozważnie!
