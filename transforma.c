#include "transforma.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void printMatrix(int **matrix, int simbolos, int num_nuevos_estados, int num_estados_originales, AFND *p_afnd)
{
  int i = 0, j = 0, k = 0;
  printf("lambda_____________\t");
  for (i = 0; i < (simbolos - 1); i++)
  {
    printf("%s_____________\t", AFNDSimboloEn(p_afnd, i));
  }

  printf("\n");
  for (i = 0; i < num_nuevos_estados; i++)
  {

    for (j = 0; j < simbolos; j++)
    {
      char newstate[100] = {0};
      for (k = 0; k < num_estados_originales; k++)
      {

        if (matrix[simbolos * i + j][k])
        {
          strcat(newstate, AFNDNombreEstadoEn(p_afnd, k));
        }
      }
      printf("/%s/\t\t\t\t", newstate);
    }
    printf("\n");
  }

  printf("\n\n");
}

int estadoFinal(int *estados, int num_estados_originales, AFND *original)
{
  int i = 0;
  for (i = 0; i < num_estados_originales; i++)
  {
    if (estados[i])
    {
      int tipo = AFNDTipoEstadoEn(original, i);
      if (tipo == FINAL || tipo == INICIAL_Y_FINAL)
      {
        return 1;
      }
    }
  }
  return 0;
}

int existeTransicion(int *estados, int num_estados_originales)
{
  int i = 0;
  for (i = 0; i < num_estados_originales; i++)
  {
    if (estados[i])
    {
      return 1;
    }
  }
  return 0;
}

char *nombreEstadoAFNDNuevo(int *estados, int num_estados_originales, AFND *p_afnd)
{
  int i = 0;
  char *nombre = (char *)malloc(100 * sizeof(char));

  for (i = 0; i < 100; i++)
  {
    nombre[i] = 0;
  }

  for (i = 0; i < num_estados_originales; i++)
  {
    if (estados[i])
    {
      strcat(nombre, AFNDNombreEstadoEn(p_afnd, i));
    }
  }

  return nombre;
}

AFND *matrix2AFND(int **matrix, int simbolos, int num_nuevos_estados, int num_estados_originales, AFND *p_afnd)
{
  int i = 0, j = 0;
  AFND *afndNew = AFNDNuevo("afndNew", num_nuevos_estados, simbolos - 1);
  /*Símbolos*/
  for (i = 0; i < simbolos - 1; i++)
  {
    AFNDInsertaSimbolo(afndNew, AFNDSimboloEn(p_afnd, i));
  }
  /*Estado Inicial*/
  char *nombre = nombreEstadoAFNDNuevo(matrix[0], num_estados_originales, p_afnd);
  if (estadoFinal(matrix[0], num_estados_originales, p_afnd))
  {
    AFNDInsertaEstado(afndNew, nombre, INICIAL_Y_FINAL);
  }
  else
  {
    AFNDInsertaEstado(afndNew, nombre, INICIAL);
  }
  free(nombre);
  /*Resto de Estados*/
  for (i = 1; i < num_nuevos_estados; i++)
  {
    nombre = nombreEstadoAFNDNuevo(matrix[simbolos * i], num_estados_originales, p_afnd);
    if (estadoFinal(matrix[simbolos * i], num_estados_originales, p_afnd))
    {
      AFNDInsertaEstado(afndNew, nombre, FINAL);
    }
    else
    {
      AFNDInsertaEstado(afndNew, nombre, NORMAL);
    }
    free(nombre);
  }
  /*Transiciones*/
  for (i = 0; i < num_nuevos_estados; i++)
  {
    for (j = 1; j < simbolos; j++)
    {
      if (existeTransicion(matrix[i * simbolos + j], num_estados_originales))
      {
        char *nombre1 = nombreEstadoAFNDNuevo(matrix[i * simbolos], num_estados_originales, p_afnd);
        char *nombre2 = nombreEstadoAFNDNuevo(matrix[i * simbolos + j], num_estados_originales, p_afnd);
        AFNDInsertaTransicion(afndNew, nombre1, AFNDSimboloEn(p_afnd, j - 1), nombre2);
        free(nombre1);
        free(nombre2);
      }
    }
  }

  return afndNew;
}

AFND *AFNDTransforma(AFND *p_afnd)
{
  /* Inicializacion de la estructura intermedia*/
  AFND *p_afndaux = NULL;
  int simbolos = AFNDNumSimbolos(p_afnd) + 1;
  int num_estados_originales = AFNDNumEstados(p_afnd);
  int num_nuevos_estados = 1;
  int **x = (int **)malloc(simbolos * sizeof(int *));
  int i = 0, j = 0, k = 0, n = 0, transicion_simbolo = 0, iguales = 1, no_encontrado = 1, flag;
  p_afndaux = AFNDCierraLTransicion(p_afnd);
  int estado_actual = AFNDIndiceEstadoInicial(p_afnd);

  for (i = 0; i < simbolos; i++)
  {
    x[i] = (int *)malloc(num_estados_originales * sizeof(int));
    for (j = 0; j < num_estados_originales; j++)
    {
      x[i][j] = 0;
    }
  }

  /* Bucle calculo valores de cada estado*/
  while (estado_actual < num_nuevos_estados)
  {

    /* Transiciones lambda estado incial*/
    /* Estado inicial donde viene el q0 dado*/
    if (estado_actual == 0)
    {
      for (k = 0; k < num_estados_originales; k++)
      {

        x[simbolos * estado_actual][k] = AFNDCierreLTransicionIJ(p_afndaux, estado_actual, k); /* Solo se mira q0 */
      }
    }

    /* Cualquier otro caso. Este y el anterior sobrarían a partir de estado nuevo 1 duda1. Deberia ya estar añadido */
    if (estado_actual != 0)
    {
      for (k = 0; k < num_estados_originales; k++)
      {

        if (x[simbolos * estado_actual][k])
        { /* Se miran las transiciones lambda de dicho estado y se añaden los estados a los que llega */
          for (i = 0; i < num_estados_originales; i++)
          {
            if (AFNDCierreLTransicionIJ(p_afndaux, k, i))
            {
              x[simbolos * estado_actual][i] = 1;
            }
          }
        }
      }
    }

    /* Calculo del conjunto estados por cada simbolo */
    for (k = 0; k < (simbolos - 1); k++)
    {

      /* Se recorren el vector lambda y aquellos estados que contenga se vera si existe transicion para el determinado simbolo*/
      for (j = 0; j < num_estados_originales; j++)
      {
        /* Se compureba si esta en la clausura lambda el estado */
        if (x[simbolos * estado_actual][j])
        {
          /* Se hallan las transiciones para ese estado y simbolos */
          for (i = 0; i < num_estados_originales; i++)
          {
            transicion_simbolo = 0;
            transicion_simbolo = AFNDTransicionIndicesEstadoiSimboloEstadof(p_afndaux, j, k, i);
            /* Si hay transicion se añade */
            if (transicion_simbolo)
            {
              x[simbolos * estado_actual + k + 1][i] = 1; /* +1 debido a la columna de lambda*/
              /* Se añaden las transiciones lambda desde el nuevo estado al que se puede llegar */
              for (n = 0; n < num_estados_originales; n++)
              {
                if (AFNDCierreLTransicionIJ(p_afndaux, i, n))
                {
                  x[simbolos * estado_actual + k + 1][n] = 1;
                }
              }
            }
          }
        }
      }
    }

    /* Añadir los nuevos estados */
    /* Se recorren los simbolos y se va comprobando si contienen un vector diferente a los ya persistentes */
    for (k = 0; k < (simbolos - 1); k++)
    {
      no_encontrado = 1;
      for (j = 0; j < num_nuevos_estados; j++)
      { /* Se ve si coincide con alguno de los estados */
        iguales = 1;
        for (i = 0; i < num_estados_originales; i++)
        {
          /* Estados qx comparandose con estados contenidos en el simbolo k de esta fila */
          if (x[simbolos * j][i] != x[estado_actual * simbolos + k + 1][i])
          {
            iguales = 0; /* No son iguales este par */
          }
        }
        if (iguales)
        {
          no_encontrado = 0;
        } /* Si un par son iguales, se encontró */
      }
      if (no_encontrado)
      {
        flag = 0;
        no_encontrado = 0;
        for (n = 0; n < num_estados_originales; n++)
        {
          if (x[(estado_actual * simbolos) + k + 1][n])
          {
            flag = 1;
          }
        }
        if (flag)
        {
          no_encontrado = 1;
        }
      }
      /* Se añade en caso de no encontrarse */
      if (no_encontrado)
      {
        num_nuevos_estados++;
        x = (int **)realloc(x, num_nuevos_estados * simbolos * sizeof(int *));
        for (i = 0; i < simbolos; i++)
        {
          x[(simbolos * (num_nuevos_estados - 1)) + i] = (int *)malloc(num_estados_originales * sizeof(int));
          for (j = 0; j < num_estados_originales; j++)
          {
            x[(simbolos * (num_nuevos_estados - 1)) + i][j] = 0;
          }
        }
        for (i = 0; i < num_estados_originales; i++)
        {
          x[simbolos * (num_nuevos_estados - 1)][i] = x[(estado_actual * simbolos) + k + 1][i];
        }
      }
    }
    estado_actual++;
  }

  printMatrix(x, simbolos, num_nuevos_estados, num_estados_originales, p_afnd);
  AFND *transformedAFND = matrix2AFND(x, simbolos, num_nuevos_estados, num_estados_originales, p_afnd);

  for (i = 0; i < (num_nuevos_estados * simbolos); i++)
  {
    free(x[i]);
  }
  free(x);

  return transformedAFND;
}