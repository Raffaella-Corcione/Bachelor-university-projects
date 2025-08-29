/* RAFFAELLA CORCIONE, mat. 215144
 * PROVA FINALE DI ALGORITMI E STRUTTURE DATI a.a. 2023/2024 - POLITECNICO DI MILANO
 * */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define ALPHA (sqrt(5)-1)/2

/* per strutturare il magazzino degli ingredienti ho scelto una hash table a indirizzamento aperto. Ogni cella occupata della tabella corrisponde ad un ingrediente diverso (struct ingr_mag), mentre i lotti di ciascun ingrediente (struct lotto) sono organizzati come un albero rosso-nero secondo la loro data di scadenza.
 * */
typedef struct l_scad{
    int qt; //quantità contenuta in questo lotto
    int scad; //scadenza di questo lotto
    struct l_scad* left; //altri campi necessari per la struttura dell'albero
    struct l_scad* right;
    struct l_scad* parent;
    char color;
} lotto;

typedef struct ingr{ //ingrediente nel magazzino
    char* nome; //nome dell'ingrediente (è la chiave usata nella funzione di hash)
    lotto* lotti; //puntatore alla radice dell'albero dei lotti di questo ingrediente
    lotto* min; //puntatore al lotto con scadenza minima presente nell'albero
    lotto* nil; //puntatore all'unica foglia dell'albero
    int tot; //quantità totale presente di questo ingrediente nel magazzino
} ingr_mag;

/* per il catalogo delle ricette ho scelto una struttura simile alla precedente (hash table con closed hashing). Ogni cella occupata rappresenta una ricetta (struct ricetta), a cui corrisponde una lista di ingredienti e rispettive quantità (struct ingr_ric). Poiché molte ricette possono avere degli ingredienti in comune, i nomi degli ingredienti (struct lista_nomi_ing) sono tutti memorizzati in un'altra struttura dati così da evitare i duplicati.
 * */

typedef struct l_ing{ //struttura che contiene i nomi degli ingredienti usati nelle ricette. Organizzazione a hash table con open hashing: la chiave è determinata dalla prima lettera del nome e ogni bucket contiene tutti gli ingredienti con la stessa iniziale.
	char* ing; //stringa allocata dinamicamente contenente il nome per esteso
	struct l_ing* next; //puntatore al prossimo nome del bucket
} lista_nomi_ing;

typedef struct ing_qt_l{ //coppia ingrediente-quantità per una ricetta
	char* nome_ing; //puntatore ad una stringa contenuta in una struct lista_nomi_ing
	int qt;
        ingr_mag* ing; //puntatore all'ingrediente nel magazzino (non sempre valido)
	struct ing_qt_l* next;
} ingr_ric; //le componenti delle ricette sono organizzate a lista semplicemente concatenata in quanto nella preparazione di un ordine è sempre necessaria la lettura della ricetta completa e non è necessario un ordinamento particolare.

typedef struct ric{ //una ricetta è una lista di coppie ingrediente-quantità con un nome
	char* nome; //nome per esteso della ricetta
	ingr_ric* lista_ingredienti;
        int min_non_cuc; //campo utilizzato nella preparazione degli ordini a seguito di un rifornimento. Vedi la corrispondente sezione del main
} ricetta;

typedef struct ord{ //un dolce
	int t_arrivo; //istante di arrivo dell'ordine
	int qt; //numero di dolci contenuti nell'ordine
	int peso; //peso totale dell'ordine calcolato al momento della sua preparazione
        ricetta* ric; //puntatore alla ricetta corrispondente nel ricettario. Da essa si ricava anche il nome del dolce
        struct ord* next; //per le strutture delle code di attesa
} ordine;

//parametri per le tabelle hash di ricettario e magazzino:
int mr = 1024; //dimensione iniziale del ricettario
float load_factor_r = 0.0; //fattore di carico del ricettario
int mm = 1024; //dimensione iniziale del magazzino ingredienti
float load_factor_m = 0.0; //fattore di carico del magazzino

// PROTOTIPI
int hash(char*, int); //funzione di hash per ricettario e magazzino
int quad_probe(int, int, int); //funzione per ispezione di ricettario e magazzino
ricetta* ins_ingr_ric(ricetta*, char*, int, ingr_mag**, lista_nomi_ing**); //crea e inserisce un ingrediente in una ricetta
ricetta* crea_ricetta(ricetta*, char*, ingr_mag**, lista_nomi_ing**); //crea una ricetta nuova
ricetta** aggiungi_ricetta(ricetta**, ingr_mag**, lista_nomi_ing**); //funzione associata al comando della pasticceria "aggiungi_ricetta"
ricetta* elimina_ricetta(ricetta*); //elimina una ricetta dal ricettario
ricetta** rimuovi_ricetta(ricetta**, ordine*, ordine*); //associata al comando "rimuovi_ricetta"
ricetta** distruggi_ricettario(ricetta**); //distrugge un ricettario deallocando tutte le sue ricette e il ricettario stesso
ricetta** rialloca_ricettario(ricetta**); //rialloca il ricettario in caso di tabella hash troppo piena
int cerca_ricetta(ricetta**, char*); //cerca una ricetta nel ricettario
ingr_mag** rifornimento(ingr_mag**, int); //associata al comando "rifornimento"
ingr_mag* inserisci_lotto(ingr_mag*, int, int); //inserisce un lotto nuovo nell'albero dei lotti di un ingrediente
lotto* riparaRBInserisci(lotto*, lotto*, char*);
lotto* leftRotate(lotto*, lotto*);
lotto* rightRotate(lotto*, lotto*);
ingr_mag* cancella_lotto_min(ingr_mag*); //cancella il lotto con scadenza più prossima
lotto* successore(lotto*, lotto*); //successore di un nodo in un bst
lotto* riparaRBCancella(lotto*, lotto*);
lotto* RBTransplant(lotto*, lotto*, lotto*);
lotto* distruggi_albero(lotto*, lotto*); //distrugge l'albero dei  lotti per un determinato ingrediente
ingr_mag* crea_ingrediente(ingr_mag*, char*); //alloca un nuovo ingrediente con albero dei lotti inizializzato e vuoto
ingr_mag** distruggi_magazzino(ingr_mag**); //distrugge il magazzino deallocando tutti gli ingredienti e il magazzino stesso
ingr_mag** rialloca_magazzino(ingr_mag**); //riallocazione in caso di hash table troppo piena per il magazzino
int verifica_se_cucinabile(ingr_ric*, ingr_mag**, int, int); //verifica se il magazzino contiene abbastanza ingredienti per preparare un ordine. Elimina eventualmente anche dei lotti scaduti
ordine* cucina_ordine(ordine*, ingr_ric*); //prepara un ordine
ordine* ins_in_ord_cresc(ordine*, ordine*); //inserimento in ordine crescente per lista semplice di ordini
ordine* ins_in_fondo(ordine*, ordine*, ordine*); //inserimento in fondo per lista semplice di ordini
int cerca_ingrediente(ingr_mag**, char*); //cerca un ingrediente nel magazzino
ordine* distruggi_lista(ordine*); //distrugge una lista di ordini
ordine* corriere(ordine*, int); //seleziona gli ordini da spedire all'arrivo del corriere
void sort(ordine**, int, int); //ordinamento di un vettore di ordini secondo il merge sort in ordine decrescente di peso
void merge(ordine**, int, int, int);
lista_nomi_ing** distruggi_catalogo(lista_nomi_ing**); //distrugge il catalogo dei nomi degli ingredienti

ricetta** aggiungi_ricetta(ricetta** ricettario, ingr_mag** magazzino, lista_nomi_ing** catalogo){
    char nome_[256]; //secondo la specifica, i nomi delle ricette e degli ingredienti sono lunghi al più 255 caratteri. Prevedo un carattere in più per il terminatore della stringa.
    if(scanf("%s", nome_) == 1){ //leggo il nome della ricetta da aggiungere
        if(ricettario[cerca_ricetta(ricettario, nome_)] != NULL){ //ricetta già presente
            printf("ignorato\n");
            char sep;
            char nuovo_ingr[256];
            int nuova_qt;
            while(scanf("%c", &sep) == 1 && sep != '\n' && scanf("%s %d", nuovo_ingr, &nuova_qt) == 2){
                //ciclo che serve solo a leggere tutti gli ingredienti della ricetta che vanno ignorati
            }
            return ricettario;
        }
        int h = hash(nome_, mr); //calcola la posizione in cui la nuova ricetta dovrebbe essere inserita
        int k = h;
        int i = 0;
        while(ricettario[k] != NULL && strcmp(ricettario[k]->nome, "+") != 0){ //attua la procedura di ispezione nel caso la cella sia occupata da un'altra ricetta o da una tombstone (identificata sia nel ricettario sia nel magazzino dal carattere '+' nel campo nome, poiché esso non fa parte dell'alfabeto a cui appartengono i nomi delle ricette e degli ingredienti secondo la specifica)
            i++;
            k = quad_probe(h, i, mr);
        }
        if(ricettario[k] == NULL) //quando si trova un posto vuoto
            load_factor_r += 1.0/mr; //aggiorna il fattore di carico
        ricettario[k] = crea_ricetta(ricettario[k], nome_, magazzino, catalogo); //crea la ricetta nuova
        if(load_factor_r >= 0.5) //si rialloca il ricettario quando la tabella è piena per metà
            ricettario = rialloca_ricettario(ricettario);
    }
    return ricettario;
}

ricetta** rimuovi_ricetta(ricetta** ricettario, ordine* att_prep, ordine* att_sped){
    char da_canc[256];
    if(scanf("%s", da_canc) == 1){ //leggo il nome della ricetta da rimuovere
        int p = cerca_ricetta(ricettario, da_canc); //cerca la ricetta secondo il suo nome nel ricettario
        if(ricettario[p] != NULL){ //se è presente
            while(att_prep != NULL){ //controlla la lista degli ordini in attesa di preparazione
                if(strcmp(att_prep->ric->nome, ricettario[p]->nome) == 0){
                    printf("ordini in sospeso\n");
                    return ricettario;
                }
                att_prep = att_prep->next;
            }
            while(att_sped != NULL){ //controlla la lista di ordini in attesa di spedizione
                if(strcmp(att_sped->ric->nome, ricettario[p]->nome) == 0){
                    printf("ordini in sospeso\n");
                    return ricettario;
                }
                att_sped = att_sped->next;
            }
            ricettario[p] = elimina_ricetta(ricettario[p]); //eseguiti i controlli, elimina la ricetta
            printf("rimossa\n");
        }
        else{
            printf("non presente\n");
        }
    }
    return ricettario;
}

ingr_mag** rifornimento(ingr_mag** magazzino, int t){
    char nome_[256]; //nome dell'ingrediente rifornito
    int q; //quantità presente nel lotto nuovo
    int s; //scadenza del lotto nuovo
    char sep = '0'; //carattere necessario a separare i gruppi "nome quantità scadenza" e a capire quando è finito l'elenco con la fine della riga
    while(scanf("%c", &sep) == 1 && sep != '\n' && scanf("%s %d %d", nome_, &q, &s) == 3){
        if(s > t){ //inserisco il lotto nuovo nel magazzino solo se non è già scaduto
            int i = cerca_ingrediente(magazzino, nome_);
            if(magazzino[i] != NULL)
                magazzino[i] = inserisci_lotto(magazzino[i], q, s); //inserisco un lotto nuovo in un ingrediente già presente nel magazzino
            else{ //devo creare un ingrediente nuovo: procedura simile ad aggiungi_ricetta, con ricerca di un posto vuoto ed eventuale riallocazione del magazzino
                int h = hash(nome_, mm);
                int k = h;
                int i = 0;
                while(magazzino[k] != NULL && strcmp(magazzino[k]->nome, "+") != 0){
                    i++;
                    k = quad_probe(h, i, mm);
                }
                if(magazzino[k] == NULL)
                    load_factor_m += 1.0/mm;
                magazzino[k] = crea_ingrediente(magazzino[k], nome_); //inizializza l'ingrediente nuovo con albero dei lotti vuoto
                magazzino[k] = inserisci_lotto(magazzino[k], q, s); //aggiungi il primo lotto appena arrivato
                if(load_factor_m >= 0.5)
                    magazzino = rialloca_magazzino(magazzino);
            }
        }
    }
    printf("rifornito\n");
    return magazzino;
}

int hash(char* str, int m){ //funzione di hash per ricettario e magazzino. Metodo della moltiplicazione, adatto a tabelle di dimensione pari a una potenza di 2.
    //non sarà a costo costante ma per poter usare tutti i bit della codifica della chiave devo leggerla tutta per forza
    unsigned int k = str[0] * m;
    for(int i=0; i<strlen(str); i++){ //per estrarre un numero intero dalla stringa rappresentante la chiave, somma tutti i valori delle codifiche ascii dei caratteri in essa presenti
        k += (unsigned int)str[i];
    }
    return (unsigned int)((uint32_t)(k*(double)ALPHA*((uint64_t)1<<32)))%m; //come dalle slide.
}

int quad_probe(int a, int i, int m){ //funzione che effettua il quadratic probing nelle hash table di ricettario e magazzino. Come dimostrato a lezione, questa funzione assicura di toccare tutte le celle per tabelle di dimensione 2^x.
	float r = a + 0.5*i + 0.5*i*i;
    return (int)r % m;
}

int cerca_ricetta(ricetta** ricettario, char* nome_){ //normale funzione di ricerca in una hash table, con calcolo della funzione di hash ed eventuale sequenza di ispezione
	int h = hash(nome_, mr);
	int k = h;
	int i = 0;
	while(ricettario[k] != NULL && strcmp(ricettario[k]->nome, nome_) != 0){
		i++;
		k = quad_probe(h, i, mr);
	}
	return k;
}

ricetta* ins_ingr_ric(ricetta* ric, char* ingr, int quant, ingr_mag** magazzino, lista_nomi_ing** catalogo){
    ingr_ric* nuovo_ing = (ingr_ric*)malloc(sizeof(ingr_ric)); //alloca una nuova struttura ingrediente per la ricetta
    int i = 0; //individua il bucket del catalogo ingredienti dove si trova o va inserito il nome ingr
    if(isdigit(ingr[0])) //iniziali 0-9: posizioni 0-9 della tabella
        i = (int)ingr[0];
    else if(97 <= (int)ingr[0] && (int)ingr[0] <= 122) //iniziali a-z: posizioni 10-35
        i = (int)ingr[0] - 87;
    else if(65 <= (int)ingr[0] && (int)ingr[0] <= 90) //iniziali A-Z: posizioni 36-61
        i = (int)ingr[0] - 29;
    else if(ingr[0] == '_') //iniziale _: posizione 62
        i = 62;
    lista_nomi_ing* punt = catalogo[i]; //primo elemento del bucket individuato
    while(punt != NULL){ //se è già presente un ingrediente con questo nome, allora il nome del nuovo ingrediente punterà ad esso, altrimenti aggiungo il nuovo nome in testa al suo bucket
        if(strcmp(ingr, punt->ing) == 0){
            nuovo_ing->nome_ing = punt->ing;
            break;
        }
        else{
            punt = punt->next;
        }
    }
    if(punt == NULL){
        lista_nomi_ing* nuovo_nome = (lista_nomi_ing*)malloc(sizeof(lista_nomi_ing));
        nuovo_nome->ing = (char*)malloc(strlen(ingr)+1);
        nuovo_nome->ing = strcpy(nuovo_nome->ing, ingr);
        nuovo_nome->next = catalogo[i];
        catalogo[i] = nuovo_nome;
        nuovo_ing->nome_ing = nuovo_nome->ing;
    }
    nuovo_ing->qt = quant; //completa l'inizializzazione del nuovo ingrediente con la quantità, il posto nel magazzino e l'aggiornamento della lista ingredienti della ricetta
    nuovo_ing->next = ric->lista_ingredienti;
    ric->lista_ingredienti = nuovo_ing;
    nuovo_ing->ing = magazzino[cerca_ingrediente(magazzino, ingr)];
    return ric; //ritorna la ricetta modificata con l'ingrediente nuovo
}

ricetta* crea_ricetta(ricetta* nuova_ric, char* nome_, ingr_mag** magazzino, lista_nomi_ing** catalogo){
    char nuovo_ingr[256];
    int nuova_qt;
    char sep; //guarda i separatori per vedere quando e' finita una riga e bisogna smettere di leggere coppie
    if(nuova_ric != NULL){ //potrebbe capitare se inserisco al posto di una tomba, cancello la ricetta vecchia che contiene '+' nel campo nome
        free(nuova_ric->nome);
        free(nuova_ric);
    }
    nuova_ric = (ricetta*)malloc(sizeof(ricetta)); //alloco la ricetta nuova
    nuova_ric->nome = (char*)malloc((strlen(nome_)+1)*sizeof(char)); //alloco il nome nuovo per la ricetta
    strcpy(nuova_ric->nome, nome_);
    nuova_ric->lista_ingredienti = NULL;
    while(scanf("%c", &sep) == 1 && sep != '\n' && scanf("%s %d", nuovo_ingr, &nuova_qt) == 2){ //inizializza la lista ingredienti
        nuova_ric = ins_ingr_ric(nuova_ric, nuovo_ingr, nuova_qt, magazzino, catalogo);
    }
    nuova_ric -> min_non_cuc = 0;
    printf("aggiunta\n");
    return nuova_ric;
}

ricetta* elimina_ricetta(ricetta* da_canc){
	//sostituisce la ricetta con una tombstone, cancellando la lista degli ingredienti
	ingr_ric* punt = da_canc->lista_ingredienti;
	while(da_canc->lista_ingredienti != NULL){
		da_canc->lista_ingredienti = da_canc->lista_ingredienti->next;
		free(punt);
		punt = da_canc->lista_ingredienti;
	}
	free(da_canc->nome);
	da_canc->nome = (char*)malloc(2*sizeof(char));
	strcpy(da_canc->nome, "+");
	return da_canc;
}

ricetta** distruggi_ricettario(ricetta** ricettario){
//dealloca tutte le ricette presenti, tutte le tombstone e infine la tabella hash stessa
    for(int i=0; i<mr; i++){
        if(ricettario[i] != NULL && strcmp(ricettario[i]->nome, "+") != 0){
            ricettario[i] = elimina_ricetta(ricettario[i]); //sostituisce prima tutte le ricette con delle tombe per avere più ordine
        }
        if(ricettario[i] != NULL && strcmp(ricettario[i]->nome, "+") == 0){
            free(ricettario[i]->nome);
            free(ricettario[i]);
            ricettario[i] = NULL;
        }
    }
    free(ricettario);
    return NULL;
}

ricetta** rialloca_ricettario(ricetta** ricettario){ //raddoppia la dimensione del ricettario quando il fattore di carico supera 1/2
    ricetta** nuovo_ricettario = (ricetta**)calloc(2*mr, sizeof(ricetta*));
    if(nuovo_ricettario == NULL){
        printf("riallocazione non andata a buon fine\n");
        return ricettario;
    }
    mr = 2*mr; //raddoppia la dimensione
    load_factor_r = 0.0; //azzera il fattore di carico
    for(int i=0; i<mr/2; i++){ //scorre tutto il vecchio ricettario per cancellare le tombe e sistemare le ricette nel ricettario nuovo calcolando la nuova posizione tramite la funzione di hash
        if(ricettario[i] != NULL && strcmp(ricettario[i]->nome, "+") != 0){
            int index = hash(ricettario[i]->nome, mr);
            int index2 = index;
            int j = 1;
            while(nuovo_ricettario[index2] != NULL){
                index2 = quad_probe(index, j, mr);
                j++;
            }
            nuovo_ricettario[index2] = ricettario[i];
            load_factor_r += 1.0/mr; //ricalcola il fattore di carico
        }
        if(ricettario[i] != NULL && strcmp(ricettario[i]->nome, "+") == 0){ //le tombe si possono eliminare e basta
            free(ricettario[i]->nome);
            free(ricettario[i]);
        }
    }
    free(ricettario);
    return nuovo_ricettario;
}

lista_nomi_ing** distruggi_catalogo(lista_nomi_ing** catalogo){ //distrugge la tabella hash e tutti i suoi bucket
    for(int i=0; i<63; i++){
        lista_nomi_ing* t = catalogo[i];
        while(catalogo[i] != NULL){
            catalogo[i] = catalogo[i]->next;
            free(t->ing);
            free(t);
            t = catalogo[i];
        }
    }
    free(catalogo);
    return NULL;
}

ingr_mag* crea_ingrediente(ingr_mag* i, char* nome_){ //crea un ingrediente nuovo con albero dei lotti vuoto
    if(i != NULL) //cancella un eventuale nome di tomba
        free(i->nome);
    if(i == NULL){
        i = (ingr_mag*)malloc(sizeof(ingr_mag)); //alloca un nuovo ingrediente
        //albero vuoto significa una sola foglia nulla
        i->lotti = (lotto*)malloc(sizeof(lotto)); //inizializza T.nil
        i->lotti->qt = -1;
        i->lotti->scad = -1;
        i->lotti->left = NULL;
        i->lotti->right = NULL;
        i->lotti->parent = NULL;
        i->lotti->color = 'b';
        i->min = NULL;
        i->nil = i->lotti;
    }
    i->nome = (char*)malloc((strlen(nome_)+1) * sizeof(char));
    strcpy(i->nome, nome_);
    i->tot = 0; //l'ingrediente non contiene ancora lotti
    return i;
}

ingr_mag** rialloca_magazzino(ingr_mag** magazzino){ //raddoppia la dimensione del magazzino quando il fattore di carico supera 1/2, analogamente a rialloca_ricettario
    ingr_mag** nuovo_magazzino = (ingr_mag**)calloc(2*mm, sizeof(ingr_mag*));
    if(nuovo_magazzino == NULL){
        printf("riallocazione non andata a buon fine\n");
        return magazzino;
    }
    mm = 2*mm;
    load_factor_m = 0.0;
    for(int i=0; i<mm/2; i++){
        if(magazzino[i] != NULL && strcmp(magazzino[i]->nome, "+") != 0){
            int index = hash(magazzino[i]->nome, mm);
            int index2 = index;
            int j = 1;
            while(nuovo_magazzino[index2] != NULL){
                index2 = quad_probe(index, j, mm);
                j++;
            }
            nuovo_magazzino[index2] = magazzino[i];
            load_factor_m += 1.0/mm;
        }
        if(magazzino[i] != NULL && strcmp(magazzino[i]->nome, "+") == 0){
            free(magazzino[i]->nome);
            free(magazzino[i]);
        }
    }
    free(magazzino);
    return nuovo_magazzino;
}

int cerca_ingrediente(ingr_mag** magazzino, char* nome_){ //analogo a cerca_ricetta
	int h = hash(nome_, mm);
	int k = h;
	int i = 0;
	while(magazzino[k] != NULL && strcmp(magazzino[k]->nome, nome_) != 0){
		i++;
		k = quad_probe(h, i, mm);
	}
	return k;
}

//tutte le funzioni riguardanti gli alberi rosso-neri rispecchiano quelle riportate sulle slide e sul libro di testo
ingr_mag* inserisci_lotto(ingr_mag* i, int q, int s){ //RBInserisci
    //la funzione richiede di aver già letto la quantità e la scadenza del lotto da inserire
    //1. cerca il lotto futuro padre del lotto da inserire
    i->tot += q;
    lotto* pre = NULL;
    lotto* cur = i->lotti;
    while(cur != i->nil){
        if(cur->scad == s){ //se è già presente un lotto con questa scadenza non serve fare altro che aggiungere la nuova quantità allo stesso
            cur->qt += q;
            return i;
        }
        pre = cur;
        if(s < cur->scad)
            cur = cur->left;
        else
            cur = cur->right;
    }
    //2. aggiungere il nodo nuovo
    lotto* nuovo_lotto = (lotto*)malloc(sizeof(lotto));
    nuovo_lotto->qt = q;
    nuovo_lotto->scad = s;
    nuovo_lotto->color = 'r';
    nuovo_lotto->left = i->nil;
    nuovo_lotto->right = i->nil;
    nuovo_lotto->parent = pre;
    if(nuovo_lotto->parent == NULL){ //ho inserito la radice
        nuovo_lotto->color = 'b';
        i->lotti = nuovo_lotto;
        i->min = nuovo_lotto;
        return i;
    }
    else{
        if(s < pre->scad)
            pre->left = nuovo_lotto;
        else
            pre->right = nuovo_lotto;
    }
    if(pre->color == 'r')
        i->lotti = riparaRBInserisci(i->lotti, nuovo_lotto, i->nome);
    //3. aggiornare il nodo minimo
    if(s < i->min->scad)
        i->min = nuovo_lotto;
    return i;
}

lotto* riparaRBInserisci(lotto* tree, lotto* x, char* nome){
    while(x != tree && x->parent->color == 'r'){
        if(x->parent == x->parent->parent->left){
            lotto* y = x->parent->parent->right; //lo zio di x
            if(y->color == 'r'){
                x->parent->color = 'b';
                y->color = 'b';
                x->parent->parent->color = 'r';
                x = x->parent->parent;
            }
            else{
                if(x == x->parent->right){
                    x = x->parent;
                    tree = leftRotate(tree, x);
                }
                x->parent->color = 'b';
                x->parent->parent->color = 'r';
                tree = rightRotate(tree, x->parent->parent);
            }
        }
        else{
            lotto* y = x->parent->parent->left; //lo zio di x
            if(y->color == 'r'){
                x->parent->color = 'b';
                y->color = 'b';
                x->parent->parent->color = 'r';
                x = x->parent->parent;
            }
            else{
                if(x == x->parent->left){
                    x = x->parent;
                    tree = rightRotate(tree, x);
                }
                x->parent->color = 'b';
                x->parent->parent->color = 'r';
                tree = leftRotate(tree, x->parent->parent);
            }
        }
    }
    tree->color = 'b';
    return tree;
}

lotto* leftRotate(lotto* tree, lotto* x){
    lotto* y = x->right;
    x->right = y->left;
    if(y->left->scad != -1)
        y->left->parent = x;
    y->parent = x->parent;
    if(x->parent == NULL)
        tree = y;
    else if(x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    y->left = x;
    x->parent = y;

    return tree;
}

lotto* rightRotate(lotto* tree, lotto* y){
    lotto* x = y->left;
    y->left = x->right;
    if(x->right->scad != -1)
        x->right->parent = y;
    x->parent = y->parent;
    if(y->parent == NULL)
        tree = x;
    else if(y == y->parent->right)
        y->parent->right = x;
    else
        y->parent->left = x;
    x->right = y;
    y->parent = x;

    return tree;
}

ingr_mag* cancella_lotto_min(ingr_mag* i){ //poiché uso i lotti dando sempre la precedenza a quelli con scadenza più prossima, mi serve solo cancellare il minimo.
    lotto* da_canc = i->min;
    i->tot = i->tot - da_canc->qt;
    i->min = successore(i->lotti, i->min);
    if(i->min == NULL){ //il minimo è l'unico nodo presente, devo cancellare l'ingrediente
        free(da_canc);
        i->lotti = i->nil; //nell'albero rimane solo la foglia nil
        free(i->nome);
        i->nome = (char*)malloc(2*sizeof(char));
        strcpy(i->nome, "+");
        i->tot = 0;
        return i;
    }
    //RBCancella ha costo costante
    lotto* y = da_canc;
    lotto* x;
    char yOriginalColor = y->color;
    x = da_canc->right; //il minimo non ha mai figlio sinistro
    i->lotti = RBTransplant(i->lotti, da_canc, da_canc->right);
    if(yOriginalColor == 'b'){
        i->lotti = riparaRBCancella(i->lotti, x);
    }
    i->lotti->color = 'b';
    free(da_canc);
    return i;
}

lotto* riparaRBCancella(lotto* tree, lotto* x){
    while(x != tree && x->color == 'b'){ //le foglie NULL sono nere
        if(x == x->parent->left){
            lotto* w = x->parent->right;
            if(w->color == 'r'){
                w->color = 'b';
                x->parent->color = 'r';
                tree = leftRotate(tree, x->parent);
                w = x->parent->right;
            }
            if(w->left->color == 'b' && w->right->color == 'b'){
                w->color = 'r';
                x = x->parent;
            }
            else{
                if(w->right->color == 'b'){
                    w->left->color = 'b';
                    w->color = 'r';
                    tree = rightRotate(tree, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = 'b';
                w->right->color = 'b';
                tree = leftRotate(tree, x->parent);
                x = tree;
            }
        }
        else{
            lotto* w = x->parent->left;
            if(w->color == 'r'){
                w->color = 'b';
                x->parent->color = 'r';
                tree = rightRotate(tree, x->parent);
                w = x->parent->left;
            }
            if(w->right->color == 'b' && w->left->color == 'b'){
                w->color = 'r';
                x = x->parent;
            }
            else{
                if(w->left->color == 'b'){
                    w->right->color = 'b';
                    w->color = 'r';
                    tree = leftRotate(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = 'b';
                w->left->color = 'b';
                tree = rightRotate(tree, x->parent);
                x = tree;
            }
        }
    }
    x->color = 'b';
    return tree;
}

lotto* RBTransplant(lotto* tree, lotto* u, lotto* v){
    if(u->parent == NULL)
        tree = v;
    else if(u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;
    v->parent = u->parent;
    return tree;
}

lotto* successore(lotto* tree, lotto* x){
    if(x->right->scad != -1){
        lotto* cur = x->right;
        while(cur->left->scad != -1)
            cur = cur->left;
        return cur;
    }
    lotto* y = x->parent;
    while(y != NULL && y->right == x){
        x = y;
        y = y->parent;
    }
    return y;
}

lotto* distruggi_albero(lotto* tree, lotto* nil){ //dealloca tutti i lotti
    if(tree->left != nil){
        tree->left = distruggi_albero(tree->left, nil);
    }
    if(tree->right != nil){
        tree->right = distruggi_albero(tree->right, nil);
    }
    free(tree);
    return NULL;
}

ingr_mag** distruggi_magazzino(ingr_mag** magazzino){ //analoga a distruggi_ricettario
    for(int i=0; i<mm; i++){
        if(magazzino[i] != NULL && strcmp(magazzino[i]->nome, "+") != 0){
            magazzino[i]->lotti = distruggi_albero(magazzino[i]->lotti, magazzino[i]->nil);
            free(magazzino[i]->nil);
            free(magazzino[i]->nome);
            free(magazzino[i]);
            magazzino[i] = NULL;
        }
        else if(magazzino[i] != NULL && strcmp(magazzino[i]->nome, "+") == 0){
            free(magazzino[i]->nome);
            free(magazzino[i]->nil);
            free(magazzino[i]);
            magazzino[i] = NULL;
        }
    }
    free(magazzino);
    return NULL;
}

int verifica_se_cucinabile(ingr_ric* p, ingr_mag** magazzino, int quant, int t){ //restituisce 1 se il dolce si può preparare con gli ingredienti attualmente presenti nel magazzino, 0 altrimenti. Effettua anche l'eliminazione dei lotti scaduti.
    while(p != NULL){ //controlla tutti gli ingredienti presenti nella lista_ingredienti della ricetta dell'ordine
        if(p->ing == NULL || strcmp(p->ing->nome, p->nome_ing) != 0) //data la costruzione del magazzino, non è detto che tutti gli ingredienti siano sempre presenti e nel posto puntato da p->ing
            p->ing = magazzino[cerca_ingrediente(magazzino, p->nome_ing)];
        while(p->ing != NULL && p->ing->min->scad <= t) {//elimino i lotti scaduti
            p->ing = cancella_lotto_min(p->ing);
            if(p->ing->tot == 0)
                return 0;
        }
        if(p->ing == NULL || p->ing->tot < (p->qt)*quant){ //un ingrediente è mancante o non sufficiente
            return 0;
        }
        p = p->next;
    }
    return 1;
}

ordine* cucina_ordine(ordine* nuovo_ordine, ingr_ric* p){
    //parametri: ordine da cucinare, lista di ingredienti componenti la ricetta
    while(p != NULL){
        //presuppongo che tutti gli ingredienti siano presenti e sufficienti, quindi tutti i puntatori interni alla ricetta siano validi
        int q = nuovo_ordine->qt * p->qt;
        nuovo_ordine->peso += nuovo_ordine->qt * p->qt; //calcola peso totale del dolce
        while(q > 0){
            if(p->ing->min->qt <= q){ //usa un lotto intero (quello con la scadenza più prossima). p->ing è sicuramente valido.
                q -= p->ing->min->qt;
                p->ing = cancella_lotto_min(p->ing);
            }
            else{ //usa solo parte di un lotto che non va quindi eliminato
                p->ing->min->qt -= q;
                p->ing->tot -= q;
                q = 0;
            }
        }
        p = p->next;
    }
    return nuovo_ordine;
}

ordine* ins_in_ord_cresc(ordine* lista, ordine* ord){ //la lista viene ordinata in ordine crescente secondo l'istante di arrivo
    if(lista == NULL || ord->t_arrivo <= lista->t_arrivo){
        ord->next = lista;
        return ord;
    }
    ordine* cur = lista;
    while(cur->next != NULL && cur->next->t_arrivo < ord->t_arrivo){
        cur = cur->next;
    }
    ord->next = cur->next;
    cur->next = ord;
    return lista;
}

ordine* ins_in_fondo(ordine* lista, ordine* coda, ordine* ord){
    if(lista == NULL){ //lista vuota
        lista = ord;
        ord->next = NULL;
        coda = ord;
        return lista;
    }
    coda->next = ord;
    coda = ord;
    return lista;
}

ordine* distruggi_lista(ordine* lista){
    ordine* cur = lista;
    while(lista != NULL){
        cur = lista->next;
        free(lista);
        lista = cur;
    }
    return NULL;
}

ordine* corriere(ordine* da_spedire, int c){
    //prima scelgo dalla lista gli ordini da caricare sul camioncino secondo l'ordine di arrivo e li metto in un vettore. Poi ordino questo vettore con il merge sort secondo il peso decrescente
    if(da_spedire == NULL){ //non ci sono ordini nella lista di attesa per spedizione. Suppongo che non ci siano mai ordini talmente pesanti da non entrare nel camioncino e bloccare tutto
        printf("camioncino vuoto\n");
        return NULL;
    }
    int l = 0; //lunghezza della porzione di vettore occupata
    ordine* punt = da_spedire;
    while(punt != NULL && punt->peso <= c){
        l++;
        c -= punt->peso;
        punt = punt->next;
    }
    ordine** v = (ordine**)calloc(l, sizeof(ordine*)); //vettore di ordini, assumendo che tutti gli ordini abbiano peso >= 1
    for(int i=0; i<l; i++){ //trasloco gli ordini scelti nel vettore
        v[i] = da_spedire;
        da_spedire = da_spedire->next; //toglie l'ordine dalla lista di quelli da spedire
    }
    sort(v, 0, l-1);
    for(int i=0; i<l; i++){
        printf("%d %s %d\n", v[i]->t_arrivo, v[i]->ric->nome, v[i]->qt);
        free(v[i]);
    }
    free(v);
    return da_spedire; //ritorna la lista aggiornata degli ordini da spedire
}

void sort(ordine** v, int p, int r){ //merge sort: mi serve un algoritmo stabile che mantenga l'ordine preesistente secondo l'istante di arrivo. Come indicata sulle slide.
    if(p < r-1){
        int q = (p+r)/2;
        sort(v, p, q);
        sort(v, q+1, r);
        merge(v, p, q, r);
    }
    else{
        if(v[p]->peso < v[r]->peso){
            ordine* tmp = v[r];
            v[r] = v[p];
            v[p] = tmp;
        }
    }
}

void merge(ordine** v, int p, int q, int r){
    int len1 = q-p+1;
    int len2 = r-q;
    ordine** L = (ordine**)malloc((len1+1)*sizeof(ordine*));
    ordine** R = (ordine**)malloc((len2+1)*sizeof(ordine*));
    for(int c=0; c<len1; c++)
        L[c] = v[p+c];
    for(int c=0; c<len2; c++)
        R[c] = v[q+1+c];
    L[len1] = (ordine*)calloc(1, sizeof(ordine));
    L[len1]->peso = 0;
    R[len2] = (ordine*)calloc(1, sizeof(ordine));
    R[len2]->peso = 0;
    int i = 0, j = 0;
    for(int k=p; k<=r; k++){
        if(L[i]->peso >= R[j]->peso){
            v[k] = L[i];
            i++;
        }
        else{
            v[k] = R[j];
            j++;
        }
    }
    free(L[len1]);
    free(R[len2]);
    free(L);
    free(R);
}

int main(){
    ricetta** ricettario = (ricetta**)calloc(mr, sizeof(ricetta*)); //allocazione di tutte le strutture dati
    ingr_mag** magazzino = (ingr_mag**)calloc(mm, sizeof(ingr_mag*));
    lista_nomi_ing** catalogo_ingredienti = (lista_nomi_ing**)calloc(63, sizeof(lista_nomi_ing*));
    ordine* att_prep = NULL; //liste per gli ordini in attesa di preparazione e di spedizione
    ordine* att_prep_tail = NULL; //gli ordini da preparare vanno inseriti sempre in coda alla lista, quindi mi conviene tenere un puntatore alla fine
    ordine* att_sped = NULL;
    int T, H; //periodicità e capienza del corriere
    if(scanf("%d %d", &T, &H) != 2)
        printf("dati mancanti\n");
    int t = 0; //istante di tempo
    char comando[17]; //tutti i comandi hanno lunghezza <= 16 caratteri
    while(scanf("%s", comando) == 1){
        if(t != 0 && t % T == 0){ //il corriere non passa all'istante iniziale
            att_sped = corriere(att_sped, H);
        }
        if(strcmp(comando, "aggiungi_ricetta") == 0)
            ricettario = aggiungi_ricetta(ricettario, magazzino, catalogo_ingredienti);
        if(strcmp(comando, "rimuovi_ricetta") == 0)
            ricettario = rimuovi_ricetta(ricettario, att_prep, att_sped);
        if(strcmp(comando, "rifornimento") == 0){
            magazzino = rifornimento(magazzino, t);
            //controllo se ci sono nuovi ordini che posso preparare. Non è necessario controllare tutta la lista di attesa: grazie al campo min_non_cuc delle ricette, posso salvare il minimo numero di dolci preparabili allo stato attuale del magazzino per ogni ricetta presente. In questo modo, se durante il controllo della lista incontro un ordine per una ricetta già incontrata e non preparata, ma per una quantità maggiore, posso evitare la chiamata a verifica_se_cucinabile e concludere direttamente che anche quest'ordine non si può preparare.
            ordine* ord = att_prep;
            ordine* pre = NULL;
            ordine* punt = NULL;
            if(ord != NULL){
                punt = ord->next;
                for(int j=0; j<mr; j++){ //prima di ogni controllo devo azzerare questo campo, poiché dopo un rifornimento la situazione del magazzino è cambiata
                    if(ricettario[j] != NULL)
                        ricettario[j]->min_non_cuc = 0;
                }
            }
            while(ord != NULL){
                if(ord->ric->min_non_cuc == 0 || ord->ric->min_non_cuc > ord->qt){
                    if(verifica_se_cucinabile(ord->ric->lista_ingredienti, magazzino, ord->qt, t) == 1){
                        ord = cucina_ordine(ord, ord->ric->lista_ingredienti); //preparo l'ordine, lo rimuovo dalla lista di attesa preparazione e lo inserisco in ordine di arrivo in quella di attesa spedizione
                        if(ord == att_prep)
                            att_prep = att_prep->next;
                        if(punt == NULL)
                            att_prep_tail = pre;
                        att_sped = ins_in_ord_cresc(att_sped, ord);
                        ord = punt;
                        if(pre != NULL)
                            pre->next = punt;
                        if(ord != NULL)
                            punt = ord->next;
                    }
                    else{
                        ord->ric->min_non_cuc = ord->qt;
                        pre = ord;
                        ord = punt;
                        if(ord != NULL)
                            punt = ord->next;
                    }
                }
                else{
                    pre = ord;
                    ord = punt;
                    if(ord != NULL)
                        punt = ord->next;
                }
            }
        }
        if(strcmp(comando, "ordine") == 0){
            char nome_[256];
            int quant;
            if(scanf("%s %d", nome_, &quant) != 2)
                printf("dati mancanti\n");
            int r = cerca_ricetta(ricettario, nome_);
            if(ricettario[r] == NULL){
                printf("rifiutato\n");
            }
            else{
                printf("accettato\n");
                ordine* nuovo_ordine = (ordine*)malloc(sizeof(ordine));
                nuovo_ordine->qt = quant;
                nuovo_ordine->t_arrivo = t; //istante corrente
                nuovo_ordine->peso = 0;
                nuovo_ordine->ric = ricettario[r]; //questo campo è sempre valido, perché durante la vita dell'ordine la ricetta non viene mai eliminata
                nuovo_ordine->next = NULL;
                ingr_ric* p = nuovo_ordine->ric->lista_ingredienti; //verifica se l'ordine si può preparare subito o se va messo in coda
                if(verifica_se_cucinabile(p, magazzino, quant, t) == 1){ //tutti gli ingredienti sono presenti, posso costruire l'ordine subito
                    nuovo_ordine = cucina_ordine(nuovo_ordine, p);
                    att_sped = ins_in_ord_cresc(att_sped, nuovo_ordine);
                }
                else{
                    att_prep = ins_in_fondo(att_prep, att_prep_tail, nuovo_ordine);
                    att_prep_tail = nuovo_ordine;
                }
            }
        }
        t++;
    }
    if(t % T == 0){ //ultima spedizione del corriere
        att_sped = corriere(att_sped, H);
    }
    //distruzione finale di tutte le strutture dati e liberazione della memoria allocata
    ricettario = distruggi_ricettario(ricettario);
    catalogo_ingredienti = distruggi_catalogo(catalogo_ingredienti);
    magazzino = distruggi_magazzino(magazzino);
    att_prep = distruggi_lista(att_prep);
    att_sped = distruggi_lista(att_sped);
    return 0;
}
