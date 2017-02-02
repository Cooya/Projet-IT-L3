/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux.
 *
 *   Copyright (C) 2015 Giuliana Bianchi, Adrien Boussicault, Thomas Place, Marc Zeitoun
 *
 *    This Library is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This Library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this Library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rationnel.h"
#include "ensemble.h"
#include "automate.h"
#include "parse.h"
#include "scan.h"
#include "outils.h"

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int yyparse(Rationnel **rationnel, yyscan_t scanner);

Rationnel *rationnel(Noeud etiquette, char lettre, int position_min, int position_max, void *data, Rationnel *gauche, Rationnel *droit, Rationnel *pere)
{
   Rationnel *rat;
   rat = (Rationnel *) malloc(sizeof(Rationnel));

   rat->etiquette = etiquette;
   rat->lettre = lettre;
   rat->position_min = position_min;
   rat->position_max = position_max;
   rat->data = data;
   rat->gauche = gauche;
   rat->droit = droit;
   rat->pere = pere;
   return rat;
}

void liberer_rationnel(Rationnel *rat) {
   if(rat->gauche)
      liberer_rationnel(rat->gauche);

   if(rat->droit)
      liberer_rationnel(rat->droit);

   free(rat);
}

Rationnel *copier_rationnel(Rationnel *rat) {
   Rationnel *rat2 = rationnel(rat->etiquette, rat->lettre, rat->position_min, rat->position_max, rat->data, NULL, NULL, rat->pere);

   switch(rat->etiquette) {
      case EPSILON:
      case LETTRE:
         break;

      case UNION:
         rat2->gauche = copier_rationnel(rat->gauche);
         rat2->gauche->pere = rat2;
         rat2->droit = copier_rationnel(rat->droit);
         rat2->droit->pere = rat2;
         break;

      case CONCAT:
         rat2->gauche = copier_rationnel(rat->gauche);
         rat2->gauche->pere = rat2;
         rat2->droit = copier_rationnel(rat->droit);
         rat2->droit->pere = rat2;
         break;

      case STAR:
         rat2->gauche = copier_rationnel(rat->gauche);
         rat2->gauche->pere = rat2;
         break;

      default:
         assert(false);
         break;
   }

   return rat2;
}

Rationnel *Epsilon()
{
   return rationnel(EPSILON, 0, 0, 0, NULL, NULL, NULL, NULL);
}

Rationnel *Lettre(char l)
{
   return rationnel(LETTRE, l, 0, 0, NULL, NULL, NULL, NULL);
}

Rationnel *Union(Rationnel* rat1, Rationnel* rat2)
{
   // Cas particulier où rat1 est vide
   if (!rat1)
      return rat2;

   // Cas particulier où rat2 est vide
   if (!rat2)
      return rat1;

   Rationnel *node = rationnel(UNION, 0, 0, 0, NULL, rat1, rat2, NULL);
   
   rat1->pere = node;
   rat2->pere = node;

   return node;
}

Rationnel *Concat(Rationnel* rat1, Rationnel* rat2)
{
   if (!rat1 || !rat2)
      return NULL;

   if (get_etiquette(rat1) == EPSILON)
      return rat2;

   if (get_etiquette(rat2) == EPSILON)
      return rat1;

   Rationnel *node = rationnel(CONCAT, 0, 0, 0, NULL, rat1, rat2, NULL);
   
   rat1->pere = node;
   rat2->pere = node;

   return node;
}

Rationnel *Star(Rationnel* rat)
{
   Rationnel *node = rationnel(STAR, 0, 0, 0, NULL, rat, NULL, NULL);
   
   rat->pere = node;

   return node;
}

bool est_racine(Rationnel* rat)
{
   return (rat->pere == NULL);
}

Noeud get_etiquette(Rationnel* rat)
{
   return rat->etiquette;
}

char get_lettre(Rationnel* rat)
{
   assert (get_etiquette(rat) == LETTRE);
   return rat->lettre;
}

int get_position_min(Rationnel* rat)
{
   assert (get_etiquette(rat) == LETTRE);
   return rat->position_min;
}

int get_position_max(Rationnel* rat)
{
   assert (get_etiquette(rat) == LETTRE);
   return rat->position_max;
}

void set_position_min(Rationnel* rat, int valeur)
{
   assert (get_etiquette(rat) == LETTRE);
   rat->position_min = valeur;
   return;
}

void set_position_max(Rationnel* rat, int valeur)
{
   assert (get_etiquette(rat) == LETTRE);
   rat->position_max = valeur;
   return;
}

Rationnel *fils_gauche(Rationnel* rat)
{
   assert((get_etiquette(rat) == CONCAT) || (get_etiquette(rat) == UNION));
   return rat->gauche;
}

Rationnel *fils_droit(Rationnel* rat)
{
   assert((get_etiquette(rat) == CONCAT) || (get_etiquette(rat) == UNION));
   return rat->droit;
}

Rationnel *fils(Rationnel* rat)
{
   assert(get_etiquette(rat) == STAR);
   return rat->gauche;
}

Rationnel *pere(Rationnel* rat)
{
   assert(!est_racine(rat));
   return rat->pere;
}

void print_rationnel(Rationnel* rat)
{
   if (rat == NULL)
   {
      printf("∅");
      return;
   }
   
   switch(get_etiquette(rat))
   {
      case EPSILON:
         printf("ε");         
         break;
         
      case LETTRE:
         printf("%c", get_lettre(rat));
         break;

      case UNION:
         printf("(");
         print_rationnel(fils_gauche(rat));
         printf(" + ");
         print_rationnel(fils_droit(rat));
         printf(")");         
         break;

      case CONCAT:
         printf("[");
         print_rationnel(fils_gauche(rat));
         printf(" . ");
         print_rationnel(fils_droit(rat));
         printf("]");         
         break;

      case STAR:
         printf("{");
         print_rationnel(fils(rat));
         printf("}*");         
         break;

      default:
         assert(false);
         break;
   }
}

Rationnel *expression_to_rationnel(const char *expr)
{
    Rationnel *rat;
    yyscan_t scanner;
    YY_BUFFER_STATE state;

    // Initialisation du scanner
    if (yylex_init(&scanner))
        return NULL;
 
    state = yy_scan_string(expr, scanner);

    // Test si parsing ok.
    if (yyparse(&rat, scanner)) 
        return NULL;
    
    // Libération mémoire
    yy_delete_buffer(state, scanner);
 
    yylex_destroy(scanner);
 
    return rat;
}

void rationnel_to_dot(Rationnel *rat, char* nom_fichier)
{
   FILE *fp = fopen(nom_fichier, "w+");
   rationnel_to_dot_aux(rat, fp, -1, 1);
}

int rationnel_to_dot_aux(Rationnel *rat, FILE *output, int pere, int noeud_courant)
{   
   int saved_pere = noeud_courant;

   if (pere >= 1)
      fprintf(output, "\tnode%d -> node%d;\n", pere, noeud_courant);
   else
      fprintf(output, "digraph G{\n");
   
   switch(get_etiquette(rat))
   {
      case LETTRE:
         fprintf(output, "\tnode%d [label = \"%c-%d\"];\n", noeud_courant, get_lettre(rat), rat->position_min);
         noeud_courant++;
         break;

      case EPSILON:
         fprintf(output, "\tnode%d [label = \"ε-%d\"];\n", noeud_courant, rat->position_min);
         noeud_courant++;
         break;

      case UNION:
         fprintf(output, "\tnode%d [label = \"+ (%d/%d)\"];\n", noeud_courant, rat->position_min, rat->position_max);
         noeud_courant = rationnel_to_dot_aux(fils_gauche(rat), output, noeud_courant, noeud_courant+1);
         noeud_courant = rationnel_to_dot_aux(fils_droit(rat), output, saved_pere, noeud_courant+1);
         break;

      case CONCAT:
         fprintf(output, "\tnode%d [label = \". (%d/%d)\"];\n", noeud_courant, rat->position_min, rat->position_max);
         noeud_courant = rationnel_to_dot_aux(fils_gauche(rat), output, noeud_courant, noeud_courant+1);
         noeud_courant = rationnel_to_dot_aux(fils_droit(rat), output, saved_pere, noeud_courant+1);
         break;

      case STAR:
         fprintf(output, "\tnode%d [label = \"* (%d/%d)\"];\n", noeud_courant, rat->position_min, rat->position_max);
         noeud_courant = rationnel_to_dot_aux(fils(rat), output, noeud_courant, noeud_courant+1);
         break;
         
      default:
         assert(false);
         break;
   }
   if (pere < 0)
      fprintf(output, "}\n");
   return noeud_courant;
}

int numeroter_rationnel_rec(Rationnel *rat, int premier_libre) { //retourne dernier utilisé
   switch(rat->etiquette) {
      case EPSILON:
      case LETTRE:
         rat->position_min = premier_libre;
         rat->position_max = premier_libre;
         break;
      case STAR:
         rat->position_min = premier_libre;
         rat->position_max = numeroter_rationnel_rec(rat->gauche, premier_libre);
         break;
      case UNION:
      case CONCAT:
         rat->position_min = premier_libre;
         int dernier_utilise_gauche = numeroter_rationnel_rec(rat->gauche, premier_libre);
         rat->position_max = numeroter_rationnel_rec(rat->droit, dernier_utilise_gauche+1);
         break;
   }
   return rat->position_max;
}

// ok
void numeroter_rationnel(Rationnel *rat)
{
   numeroter_rationnel_rec(rat, 1);
}

// ok
bool contient_mot_vide(Rationnel *rat)
{
   switch(rat->etiquette) {
      case EPSILON:
      case STAR:
         return true;
      case LETTRE:
         return false;
      case UNION:
         return contient_mot_vide(rat->gauche) || contient_mot_vide(rat->droit);
      case CONCAT:
         return contient_mot_vide(rat->gauche) && contient_mot_vide(rat->droit);
   }

   return NULL; // default
}

Ensemble *union_elements_rationnel(Rationnel *rat, Ensemble *(*action)(Rationnel *));

Ensemble *union_elements_rationnel(Rationnel *rat, Ensemble *(*action)(Rationnel *)) {
   Ensemble *eGauche = action(rat->gauche);
   Ensemble *eDroit = action(rat->droit);
   Ensemble *elements = creer_union_ensemble(eGauche, eDroit);
   liberer_ensemble(eGauche);
   liberer_ensemble(eDroit);
   return elements;
}

// ok
Ensemble *premier(Rationnel *rat)
{
   Ensemble *premiers;

   switch(rat->etiquette) {
      case EPSILON:
      case LETTRE:
         premiers = creer_ensemble(NULL, NULL, NULL);
         ajouter_element(premiers, rat->position_min);
         return premiers;
      case STAR:
         return premier(rat->gauche);
      case UNION:
         return union_elements_rationnel(rat, premier);
      case CONCAT:
         if(contient_mot_vide(rat->gauche)) // Effacable
            return union_elements_rationnel(rat, premier);
         else
            return premier(rat->gauche);
   }

   return NULL; // default
}

// ok
Ensemble *dernier(Rationnel *rat)
{
   Ensemble *derniers;

   switch(rat->etiquette) {
      case EPSILON:
      case LETTRE:
         derniers = creer_ensemble(NULL, NULL, NULL);
         ajouter_element(derniers, rat->position_min);
         return derniers;
      case STAR:
         return dernier(rat->gauche);
      case UNION:
         return union_elements_rationnel(rat, dernier);
      case CONCAT:
         if(contient_mot_vide(rat->droit)) // Effacable
            return union_elements_rationnel(rat, dernier);
         else
            return dernier(rat->droit);
   }

   return NULL; // default
}

Rationnel *suivant_chercher_position(Rationnel *node, int position);
Ensemble *produire_suivants_pere(Rationnel *rat, Rationnel *node);
Ensemble *produire_suivants(Rationnel *rat, Rationnel *node, Rationnel *node_precedent);

Rationnel *suivant_chercher_position(Rationnel *node, int position)
{
   if(node->position_min == position && node->position_max == position)
      return node;

   if(node->gauche && node->gauche->position_max >= position)
      return suivant_chercher_position(node->gauche, position);

   if(node->droit && node->droit->position_max >= position)
      return suivant_chercher_position(node->droit, position);

   return NULL;
}

Ensemble *produire_suivants_pere(Rationnel *rat, Rationnel *node) {
   if(node != rat)
      return produire_suivants(rat, node->pere, node);
   else
      return creer_ensemble(NULL, NULL, NULL);
}

Ensemble *produire_suivants(Rationnel *rat, Rationnel *node, Rationnel *node_precedent) {
   switch(node->etiquette) {
      case EPSILON:
      case LETTRE:
      case UNION:
         //printf("lettre : %c pere : %p\n", node->lettre, node->pere);
         return produire_suivants_pere(rat, node);
      case STAR: {
         Ensemble *fils_premiers = premier(node->gauche);
         Ensemble *pere_suivants = produire_suivants_pere(rat, node);
         Ensemble *elements = creer_union_ensemble(fils_premiers, pere_suivants);
         liberer_ensemble(fils_premiers);
         liberer_ensemble(pere_suivants);
         return elements;
      }
      case CONCAT:
         if(node_precedent == node->gauche) {
            if(contient_mot_vide(node->droit)) {
               Ensemble *fils_droit_premiers = premier(node->droit);
               Ensemble *pere_suivants = produire_suivants_pere(rat, node);
               Ensemble *elements = creer_union_ensemble(fils_droit_premiers, pere_suivants);
               liberer_ensemble(fils_droit_premiers);
               liberer_ensemble(pere_suivants);
               return elements;
            }
            else
               return premier(node->droit);
         }
         else
            return produire_suivants_pere(rat, node);
   }

   return NULL;
}

// ok
Ensemble *suivant(Rationnel *rat, int position)
{
   Rationnel *node = suivant_chercher_position(rat, position);

   if(!node)
      return NULL;

   return produire_suivants(rat, node, NULL);
}

void Glushkov_remplir_symboles(Rationnel *rat, char *symboles) {
   switch(rat->etiquette) {
      case EPSILON:
         break;
      case STAR:
         Glushkov_remplir_symboles(rat->gauche, symboles);
         break;
      case LETTRE:
         symboles[rat->position_min-1] = rat->lettre;
         break;
      case UNION:
      case CONCAT:
         Glushkov_remplir_symboles(rat->gauche, symboles);
         Glushkov_remplir_symboles(rat->droit, symboles);
         break;
   }
}

struct Glushkov_ajouter_transitions_args {
   Automate *automate;
   char *symboles;
   int origine;
   Ensemble *nouveaux_etats;
   bool modification;
};

void Glushkov_ajouter_transitions(const intptr_t element, void *data) {
   struct Glushkov_ajouter_transitions_args *args = (struct Glushkov_ajouter_transitions_args *)data;

   if(!est_une_transition_de_l_automate(args->automate, args->origine, args->symboles[element - 1], element)) {
      if(!est_un_etat_de_l_automate (args->automate, element))
         ajouter_element(args->nouveaux_etats, element);
      ajouter_transition(args->automate, args->origine, args->symboles[element - 1], element);
      args->modification = true;
   }
}

struct Glushkov_gerer_etat_args {
   Automate *automate;
   char *symboles;
   Rationnel *rat;
   bool premiereBoucle;
   Ensemble *nouveaux_etats;
   bool modification;
};

void Glushkov_gerer_etat(const intptr_t element, void *data) { // retourne les nouveaux états créés
   struct Glushkov_gerer_etat_args *args = (struct Glushkov_gerer_etat_args *)data;

   Ensemble *etats;

   if(args->premiereBoucle) {
      etats = premier(args->rat);
      printf("premiers : \n");
      print_ensemble(etats, NULL);
      printf("\n");
   }
   else
      etats = suivant(args->rat, (int)element);

   struct Glushkov_ajouter_transitions_args transitions_args;
   transitions_args.automate = args->automate;
   transitions_args.symboles = args->symboles;
   transitions_args.origine = (int)element;
   transitions_args.nouveaux_etats = args->nouveaux_etats;
   transitions_args.modification = false;

   pour_tout_element(etats, Glushkov_ajouter_transitions, &transitions_args);

   args->modification = transitions_args.modification;

   liberer_ensemble(etats);
}

void Glushkov_ajouter_final(const intptr_t element, void *data) {
   ajouter_etat_final((Automate *)data, element);
}

/*
Automate *Glushkov(Rationnel *rat)
{
   Automate *automate = creer_automate();

   //print_rationnel(rat);

   numeroter_rationnel(rat);

   int nombre_symboles = rat->position_max;
   char *symboles = malloc(nombre_symboles*sizeof(char));
   Glushkov_remplir_symboles(rat, symboles);

   ajouter_etat_initial(automate, 0);

   struct Glushkov_gerer_etat_args args;
   args.automate = automate;
   args.symboles = symboles;
   args.rat = rat;
   args.premiereBoucle = true;
   args.nouveaux_etats = creer_ensemble(NULL, NULL, NULL);

   Glushkov_gerer_etat(0, &args);

   args.premiereBoucle = false;

   while(args.modification) {
      args.modification = false;

      Ensemble *etats = copier_ensemble(args.nouveaux_etats);
      vider_ensemble(args.nouveaux_etats);
      print_ensemble(etats, NULL);
      
      pour_tout_element(etats, Glushkov_gerer_etat, &args);

      liberer_ensemble(etats);
   }

   liberer_ensemble(args.nouveaux_etats);
   free(symboles);

   Ensemble *finaux = dernier(rat);

   if(contient_mot_vide(rat))
      ajouter_element(finaux, 0);

   pour_tout_element(finaux, Glushkov_ajouter_final, automate);

   liberer_ensemble(finaux);

   print_automate(automate);
   printf("\n");

   return automate;
}*/

typedef struct glk_data {
   Automate *automate;
   char *symboles;
   int origine;
} glk_data;

void glk_ajouter_final(const intptr_t element, void *data) {
   ajouter_etat_final((Automate *) data, (int) element);
}

void glk_ajouter_transition(const intptr_t element, void *data) {
   glk_data *gld = (glk_data *) data;
   ajouter_transition(gld->automate, gld->origine, gld->symboles[((int) element)-1], (int) element);
}

Automate *Glushkov(Rationnel *rat) {
   Automate *automate = creer_automate();
   numeroter_rationnel(rat);

   int nb_symboles = rat->position_max;
   char *symboles = malloc(sizeof(char) * nb_symboles);
   Glushkov_remplir_symboles(rat, symboles);

   ajouter_etat_initial(automate, 0);
   if(contient_mot_vide(rat))
      ajouter_etat_final(automate, 0);

   glk_data gld;
   gld.automate = automate;
   gld.symboles = symboles;
   gld.origine = 0; 

   Ensemble *initiaux = premier(rat);
   pour_tout_element(initiaux, glk_ajouter_transition, &gld);
   Ensemble *finaux = dernier(rat);
   pour_tout_element(finaux, glk_ajouter_final, automate);

   for(int i = 1; i <= nb_symboles; i++) {
      gld.origine = i;
      Ensemble *suivants = suivant(rat, i);
      pour_tout_element(suivants, glk_ajouter_transition, &gld);
      liberer_ensemble(suivants);
   }

   liberer_ensemble(finaux);
   liberer_ensemble(initiaux);
   free(symboles);

   return automate;
}

/* La fonction devrait etre dans automate.c mais automate.h ne peut pas etre modifié */
Automate *automate_complementaire(const Automate *automate) {
   // creer_automate_deterministe() fait aussi la complétion
   Automate *a = creer_automate_deterministe(automate);

   const Ensemble *etats = get_etats(a);

   Ensemble *nouveaux_finaux = creer_ensemble(NULL, NULL, NULL);

   Ensemble_iterateur it = premier_iterateur_ensemble(etats);

   while(!iterateur_ensemble_est_vide(it)) {
      int elt = (int)get_element(it);

      if(!est_un_etat_final_de_l_automate(a, elt))
         ajouter_element(nouveaux_finaux, elt);

      it = iterateur_suivant_ensemble(it);
   }

   liberer_ensemble(a->finaux);
   a->finaux = nouveaux_finaux;

   return a;
}

bool meme_langage_rat (Rationnel *r1, Rationnel *r2)
{
   // a1 et a2 respectivement automates représentant les expression rationnelles expr1 et expr2
   Automate *a1 = Glushkov(r1);
   Automate *a2 = Glushkov(r2);

   if(comparer_ensemble(get_alphabet(a1), get_alphabet(a2)) != 0) {
      liberer_automate(a1);
      liberer_automate(a2);
      return false;
   }

   printf("A1 :\n");
   print_automate(a1);
   printf("\n");

   printf("A2 :\n");
   print_automate(a2);
   printf("\n");

   // a2c l'automate complémentaire de a2
   Automate *a2c = automate_complementaire(a2);

   printf("A2c :\n");
   print_automate(a2c);
   printf("\n");

   liberer_automate(a2);

   // a l'intersection de a1 et a2c
   Automate *a = creer_intersection_des_automates(a1, a2c);

   printf("A :\n");
   print_automate(a);
   printf("\n");

   liberer_automate(a1);
   liberer_automate(a2c);

   // on cherche a voir si aucun état final n'est accessible depuis un état initial

   Ensemble_iterateur it = premier_iterateur_ensemble(get_initiaux(a));

   while(!iterateur_ensemble_est_vide(it)) {
      int elt = (int)get_element(it);

      Ensemble *accessibles = etats_accessibles(a, elt);

      Ensemble_iterateur it2 = premier_iterateur_ensemble(accessibles);

      while(!iterateur_ensemble_est_vide(it2)) {
         if(est_un_etat_final_de_l_automate(a, (int)get_element(it2))) {
            liberer_ensemble(accessibles);
            liberer_automate(a);
            return false;
         }

         it2 = iterateur_suivant_ensemble(it2);
      }

      liberer_ensemble(accessibles);

      it = iterateur_suivant_ensemble(it);
   }

   liberer_automate(a);
   return true;
}

bool meme_langage (const char *expr1, const char* expr2) {
   Rationnel *r1 = expression_to_rationnel(expr1);
   Rationnel *r2 = expression_to_rationnel(expr2);

   bool res = meme_langage_rat(r1, r2);

   liberer_rationnel(r1);
   liberer_rationnel(r2);

   return res;
}

void systeme_ajouter_transition(int origine, char lettre, int fin, void *data) {
   Systeme systeme = (Systeme)data;

   if(systeme[origine][fin])
      systeme[origine][fin] = Union(systeme[origine][fin], Lettre(lettre));
   else
      systeme[origine][fin] = Lettre(lettre);
}

Systeme systeme(Automate *automate)
{
   unsigned int taille_automate = taille_ensemble(get_etats(automate));

   Systeme systeme = malloc(taille_automate*sizeof(Rationnel **));
   for(unsigned int i=0; i<taille_automate; i++)
      systeme[i] = calloc(taille_automate+1, sizeof(Rationnel *));

   pour_toute_transition(automate, systeme_ajouter_transition, systeme);

   Ensemble_iterateur it = premier_iterateur_ensemble(get_finaux(automate));

   while(!iterateur_ensemble_est_vide(it)) {
      int final = (int)get_element(it);

      systeme[final][taille_automate] = Epsilon();

      it = iterateur_suivant_ensemble(it);
   }

   return systeme;
}

void liberer_systeme(Systeme systeme, int taille_automate) {
   for(int i=0; i<taille_automate; i++) {
      for(int j=0; j<taille_automate+1; j++)
         if(systeme[i][j])
            liberer_rationnel(systeme[i][j]);
      free(systeme[i]);
   }
   free(systeme);
}

void print_ligne(Rationnel **ligne, int n)
{
   for (int j = 0; j <=n; j++)
      {
         print_rationnel(ligne[j]);
         if (j<n)
            printf("X%d\t+\t", j);
      }
   printf("\n");
}

void print_systeme(Systeme systeme, int n)
{
   for (int i = 0; i <= n-1; i++)
   {
      printf("X%d\t= ", i);
      print_ligne(systeme[i], n);
   }

   printf("\n");
}

Rationnel **resoudre_variable_arden(Rationnel **ligne, int numero_variable, int n)
{
   if(ligne[numero_variable]) {
      //printf("N var : %d\n", numero_variable);
      Rationnel *star = Star(ligne[numero_variable]);
      ligne[numero_variable] = NULL;

      for(int i=0; i<n; i++)
         if(ligne[i])
            ligne[i] = Concat(copier_rationnel(star), ligne[i]);
         else if(i == n-1)
            ligne[i] = copier_rationnel(star);


      liberer_rationnel(star);
   }

   return ligne;
}

Rationnel **substituer_variable(Rationnel **ligne, int numero_variable, Rationnel **valeur_variable, int n)
{
   if(ligne[numero_variable]) {
      Rationnel *operande = ligne[numero_variable];

      /*if(valeur_variable[numero_variable])
         valeur_variable = resoudre_variable_arden(valeur_variable, numero_variable, n);*/

      ligne[numero_variable] = NULL;

      for(int i=0; i<n; i++) {
         if(/*i != numero_variable && */valeur_variable[i]) {
            Rationnel *res = Concat(copier_rationnel(operande), copier_rationnel(valeur_variable[i]));

            if(ligne[i])
               ligne[i] = Union(ligne[i], res);
            else
               ligne[i] = res;
         }
      }

      liberer_rationnel(operande);
   }

   return ligne;
}

Systeme resoudre_systeme(Systeme systeme, int n)
{
   printf("Initialisation : \n");
   print_systeme(systeme, n);

   for(int i=n-1; i>=0; i--) {
      systeme[i] = resoudre_variable_arden(systeme[i], i, n+1);

      printf("Arden sur %d : \n", i);
      print_systeme(systeme, n);

      for(int j=0; j<i; j++) {
         substituer_variable(systeme[j], i, systeme[i], n+1);

         printf("Substitution de %d dans %d: \n", i, j);
         print_systeme(systeme, n);
      }
   }

   for(int i=0; i<n-1; i++) {
      for(int j=i+1; j<n; j++) {
         substituer_variable(systeme[j], i, systeme[i], n+1);

         printf("Substitution de %d dans %d: \n", i, j);
         print_systeme(systeme, n);
      }
   }

   return systeme;
}

Rationnel *Arden(Automate *automate_bis)
{
   Rationnel* expr = NULL;
   Automate *automate = creer_automate_minimal(automate_bis);
   Systeme sys = systeme(automate);
   unsigned int taille_automate = taille_ensemble(get_etats(automate));
   sys = resoudre_systeme(sys, taille_automate);

   Ensemble_iterateur it = premier_iterateur_ensemble(get_initiaux(automate));

   while(!iterateur_ensemble_est_vide(it)) {
      int initial = (int)get_element(it);

      if(expr)
         expr = Union(expr, copier_rationnel(sys[initial][taille_automate]));
      else
         expr = copier_rationnel(sys[initial][taille_automate]);

      it = iterateur_suivant_ensemble(it);
   }
   
   liberer_automate(automate);
   liberer_systeme(sys, taille_automate);

   return expr;
}

