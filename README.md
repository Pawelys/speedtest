# speedtest
Praktinė užduotis teltonikos IoT akademijai

Reikalavimai kompiliavimui bei paleidimui:
-Linux
-instaliuota libcurl biblioteka

Kompiliavimas:
1. make clean
2. make main

Paleidimas:

    Automatizuotas testas:
        ./main
    
    Geriausio serverio pagal vietove nustatymas"
        ./main location {vietoves pavadinimas, pvz:. Latvia}
    
    Individualaus serverio parsisiuntimo greicio nustatymas:
        ./main download {serverio host}
    
    Individualaus serverio siuntimo greicio nustatymas:
        ./main upload {serverio host}

Naudotojo vietoves nustatymui naudojamas ip-api.com API.

P.S automatizuoto bei vietoves testam naudojamas "hard-coded" duotas failas "speedtest_server_list.json".
