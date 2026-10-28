#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

struct Noeud {
    int valeur;
    Noeud* gauche;
    Noeud* droite;

    explicit Noeud(int v)
        : valeur(v), gauche(nullptr), droite(nullptr) {}
};

struct ABR {
    Noeud* racine = nullptr;
};

Noeud* inserer(Noeud* racine, int valeur) {
    if (racine == nullptr) {
        return new Noeud(valeur);
    }

    if (valeur < racine->valeur) {
        racine->gauche = inserer(racine->gauche, valeur);
    } else if (valeur > racine->valeur) {
        racine->droite = inserer(racine->droite, valeur);
    }
    // Si valeur == racine->valeur, le doublon est ignoré.

    return racine;
}

void inserer(ABR& arbre, int valeur) {
    arbre.racine = inserer(arbre.racine, valeur);
}

bool rechercher(const Noeud* racine, int valeur) {
    if (racine == nullptr) {
        return false;
    }
    if (valeur == racine->valeur) {
        return true;
    }
    if (valeur < racine->valeur) {
        return rechercher(racine->gauche, valeur);
    }
    return rechercher(racine->droite, valeur);
}

bool rechercher(const ABR& arbre, int valeur) {
    return rechercher(arbre.racine, valeur);
}

Noeud* minimum(Noeud* racine) {
    while (racine->gauche != nullptr) {
        racine = racine->gauche;
    }
    return racine;
}

Noeud* supprimer(Noeud* racine, int valeur) {
    if (racine == nullptr) {
        return nullptr;
    }

    if (valeur < racine->valeur) {
        racine->gauche = supprimer(racine->gauche, valeur);
    } else if (valeur > racine->valeur) {
        racine->droite = supprimer(racine->droite, valeur);
    } else {
        // Aucun enfant ou seulement un enfant.
        if (racine->gauche == nullptr) {
            Noeud* droite = racine->droite;
            delete racine;
            return droite;
        }
        if (racine->droite == nullptr) {
            Noeud* gauche = racine->gauche;
            delete racine;
            return gauche;
        }

        // Deux enfants : remplacer par le minimum du sous-arbre droit.
        Noeud* successeur = minimum(racine->droite);
        racine->valeur = successeur->valeur;
        racine->droite = supprimer(racine->droite, successeur->valeur);
    }

    return racine;
}

void supprimer(ABR& arbre, int valeur) {
    arbre.racine = supprimer(arbre.racine, valeur);
}

void afficherInfixe(const Noeud* racine) {
    if (racine == nullptr) {
        return;
    }

    afficherInfixe(racine->gauche);
    std::cout << racine->valeur << ' ';
    afficherInfixe(racine->droite);
}

void afficherArbre(const ABR& arbre) {
    if (arbre.racine == nullptr) {
        std::cout << "(arbre vide)\n";
        return;
    }
    afficherInfixe(arbre.racine);
    std::cout << '\n';
}

void afficherValeurs(const std::vector<int>& valeurs) {
    for (std::size_t i = 0; i < valeurs.size(); ++i) {
        std::cout << valeurs[i] << ' ';
        if ((i + 1) % 10 == 0) {
            std::cout << '\n';
        }
    }
    if (valeurs.empty()) {
        std::cout << "(aucune)";
    } else if (valeurs.size() % 10 != 0) {
        std::cout << '\n';
    }
}

void liberer(Noeud* racine) {
    if (racine == nullptr) {
        return;
    }
    liberer(racine->gauche);
    liberer(racine->droite);
    delete racine;
}

void testerPerformances(int nombreValeurs) {
    std::vector<int> valeurs;
    valeurs.reserve(static_cast<std::size_t>(nombreValeurs));
    for (int valeur = 1; valeur <= nombreValeurs; ++valeur) {
        valeurs.push_back(valeur);
    }

    std::mt19937 generateur(std::random_device{}());
    std::shuffle(valeurs.begin(), valeurs.end(), generateur);

    std::cout << "\nValeurs a inserer (ordre aleatoire) :\n";
    afficherValeurs(valeurs);

    // L'affichage et la generation des valeurs sont hors de la mesure.
    ABR arbreInsertion;
    const auto debutInsertion = std::chrono::steady_clock::now();
    for (int valeur : valeurs) {
        inserer(arbreInsertion, valeur);
    }
    const auto finInsertion = std::chrono::steady_clock::now();
    const double tempsInsertionMs =
        std::chrono::duration<double, std::milli>(finInsertion - debutInsertion).count();

    std::cout << "Contenu de l'ABR apres insertion (parcours infixe) :\n";
    afficherArbre(arbreInsertion);

    // Recherche de chaque valeur inseree; les valeurs et l'arbre sont deja crees.
    std::size_t nombreTrouvees = 0;
    const auto debutRecherche = std::chrono::steady_clock::now();
    for (int valeur : valeurs) {
        if (rechercher(arbreInsertion, valeur)) {
            ++nombreTrouvees;
        }
    }
    const auto finRecherche = std::chrono::steady_clock::now();
    const double tempsRechercheMs =
        std::chrono::duration<double, std::milli>(finRecherche - debutRecherche).count();

    // On construit un second arbre avant le chronometrage de la suppression.
    ABR arbreSuppression;
    for (int valeur : valeurs) {
        inserer(arbreSuppression, valeur);
    }

    std::cout << "Valeurs a supprimer (dans cet ordre) :\n";
    afficherValeurs(valeurs);

    const auto debutSuppression = std::chrono::steady_clock::now();
    for (int valeur : valeurs) {
        supprimer(arbreSuppression, valeur);
    }
    const auto finSuppression = std::chrono::steady_clock::now();
    const double tempsSuppressionMs =
        std::chrono::duration<double, std::milli>(finSuppression - debutSuppression).count();

    std::cout << "ABR apres toutes les suppressions : ";
    afficherArbre(arbreSuppression);

    std::cout << "\n===== RESULTATS DU TEST =====\n";
    std::cout << "Temps total d'insertion de " << nombreValeurs << " valeurs : "
              << tempsInsertionMs << " ms\n";
    std::cout << "Moyenne par insertion : "
              << tempsInsertionMs * 1000.0 / nombreValeurs << " microsecondes\n";
    std::cout << "Temps total de recherche de " << nombreValeurs << " valeurs : "
              << tempsRechercheMs << " ms\n";
    std::cout << "Moyenne par recherche : "
              << tempsRechercheMs * 1000.0 / nombreValeurs << " microsecondes\n";
    std::cout << "Valeurs trouvees : " << nombreTrouvees << " / " << nombreValeurs << '\n';
    std::cout << "Temps total de suppression de " << nombreValeurs << " valeurs : "
              << tempsSuppressionMs << " ms\n";
    std::cout << "Moyenne par suppression : "
              << tempsSuppressionMs * 1000.0 / nombreValeurs << " microsecondes\n";

    liberer(arbreInsertion.racine);
    arbreInsertion.racine = nullptr;
}

int main() {
    ABR arbre;
    int choix = -1;

    while (choix != 0) {
        std::cout << "\n===== MENU ABR =====\n"
                  << "1. Inserer une valeur\n"
                  << "2. Rechercher une valeur\n"
                  << "3. Supprimer une valeur\n"
                  << "4. Afficher l'ABR\n"
                  << "5. Tester le temps d'insertion, recherche et suppression\n"
                  << "0. Quitter\n"
                  << "Votre choix : ";

        if (!(std::cin >> choix)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Entree invalide. Reessayez.\n";
            continue;
        }

        int valeur;
        switch (choix) {
        case 1:
            std::cout << "Valeur a inserer : ";
            if (std::cin >> valeur) {
                if (rechercher(arbre, valeur)) {
                    std::cout << "Cette valeur existe deja; les doublons sont ignores.\n";
                } else {
                    inserer(arbre, valeur);
                    std::cout << "Valeur inseree : " << valeur << '\n';
                }
            } else {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Valeur invalide.\n";
            }
            break;

        case 2:
            std::cout << "Valeur a rechercher : ";
            if (std::cin >> valeur) {
                std::cout << (rechercher(arbre, valeur) ? "Valeur trouvee.\n"
                                                        : "Valeur absente.\n");
            } else {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Valeur invalide.\n";
            }
            break;

        case 3:
            std::cout << "Valeur a supprimer : ";
            if (std::cin >> valeur) {
                if (rechercher(arbre, valeur)) {
                    supprimer(arbre, valeur);
                    std::cout << "Valeur supprimee : " << valeur << '\n';
                } else {
                    std::cout << "Valeur absente; rien a supprimer.\n";
                }
            } else {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Valeur invalide.\n";
            }
            break;

        case 4:
            std::cout << "Contenu de l'ABR (parcours infixe) : ";
            afficherArbre(arbre);
            break;

        case 5:
            std::cout << "Combien de valeurs voulez-vous tester ? ";
            if (std::cin >> valeur && valeur > 0) {
                testerPerformances(valeur);
            } else {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Entrez un nombre entier positif.\n";
            }
            break;

        case 0:
            std::cout << "Fin du programme.\n";
            break;

        default:
            std::cout << "Choix invalide.\n";
            break;
        }
    }

    liberer(arbre.racine);
    return 0;
}
