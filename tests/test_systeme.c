/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux
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

/** @file meme_langage.c */ 

#include <automate.h>
#include <rationnel.h>
#include <ensemble.h>
#include <outils.h>
#include <parse.h>
#include <scan.h>

extern bool meme_langage_rat(Rationnel *r1, Rationnel *r2);

int test_systeme() {
  int result = 1;
    {
      Rationnel *rat = expression_to_rationnel("((a+b)*.c*.f+a.d.g.h*.(f+r*))*.g"); //"((a+b)*.c*.f+a.d.g.h*.(f+r*))*.g"
      Automate *a = Glushkov(rat);

      print_automate(a);

      Rationnel *rat2 = Arden(a);

      liberer_automate(a);

      print_rationnel(rat);
      printf("\n");
      print_rationnel(rat2);
      printf("\n");

      result = meme_langage_rat(rat, rat2);

      printf("\nMeme langage : %d\n", result);

      liberer_rationnel(rat);
      liberer_rationnel(rat2);
    }

    return result;
}

int main(int argc, char *argv[])
{
   if( ! test_systeme() )
    return 1; 
   
   return 0;
}
