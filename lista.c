#include<stdio.h>
#include<stdlib.h>

// Defini��o da estrutura da list
typedef struct{
	int type;
	double time;
	struct list * next;
} list;


// Fun��o que remove o primeiro elemento da list
list * list_pop (list * apontador)
{
	list * lap = (list *)apontador -> next;
	free(apontador);
	return lap;
}

// Fun��o que adiciona novo elemento � list, ordenando a mesma por time
list * list_add (list * apontador, double n_time, int n_type)
{
	list * lap = apontador;
	list * ap_aux, * ap_next;
	if(apontador == NULL)
	{
		apontador = (list *) malloc(sizeof (list));
		apontador -> next = NULL;
		apontador -> type = n_type;
		apontador -> time = n_time;
		return apontador;
	}
	else
	{
		if (apontador->time > n_time) {
	        ap_aux = (list *) malloc(sizeof (list));
	        ap_aux -> type = n_type;
            ap_aux -> time = n_time;
            ap_aux -> next = (struct list *) apontador;
            return ap_aux;
	    }

		ap_next = (list *)apontador -> next;
		while(apontador != NULL)
		{
			if((ap_next == NULL) || ((ap_next -> time) > n_time))
				break;
			apontador = (list *)apontador -> next;
			ap_next = (list *)apontador -> next;
		}
		ap_aux = (list *)apontador -> next;
		apontador -> next = (struct list *) malloc(sizeof (list));
		apontador = (list *)apontador -> next;
		if(ap_aux != NULL)
			apontador -> next = (struct list *)ap_aux;
		else
			apontador -> next = NULL;
		apontador -> type = n_type;
		apontador -> time = n_time;
		return lap;
	}
}

// Fun��o que imprime no ecra todos os elementos da list
void imprimir (list * apontador)
{
	if(apontador == NULL)
		printf("list vazia!\n");
	else
	{
		while(apontador != NULL)
		{
			printf("type=%d\ttime=%lf\n", apontador -> type, apontador -> time);
			apontador = (list *)apontador -> next;
		}
	}
}

// Pequeno exemplo de utiliza��o
/*int main(void)
{
	list  * list_eventos;
	int type_ev; double time_ev;
	list_eventos = NULL;
	list_eventos = adicionar(list_eventos, 1, 0.6);
	list_eventos = adicionar(list_eventos, 0, 0.4);
	list_eventos = adicionar(list_eventos, 1, 0.3);
	list_eventos = adicionar(list_eventos, 2, 0.5);
	list_eventos = adicionar(list_eventos, 1, 0.5);
	list_eventos = adicionar(list_eventos, 0, 0.2);
	list_eventos = adicionar(list_eventos, 1, 0.1);
	printf("\nlist ACTUAL\n");
	imprimir(list_eventos);

	if ( list_eventos != NULL)
	{
		type_ev = list_eventos -> type;
		time_ev = list_eventos -> time;
		list_eventos = remover(list_eventos);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\ttime=%lf\n", type_ev, time_ev);
	}
	printf("\nlist ACTUAL\n");
	imprimir(list_eventos);

	if ( list_eventos != NULL)
	{
		type_ev = list_eventos -> type;
		time_ev = list_eventos -> time;
		list_eventos = remover(list_eventos);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\ttime=%lf\n", type_ev, time_ev);
	}
	printf("\nlist ACTUAL\n");
	imprimir(list_eventos);

	if ( list_eventos != NULL)
	{
		type_ev = list_eventos -> type;
		time_ev = list_eventos -> time;
		list_eventos = remover(list_eventos);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\ttime=%lf\n", type_ev, time_ev);
	}
	printf("\nlist ACTUAL\n");
	imprimir(list_eventos);

	if ( list_eventos != NULL)
	{
		type_ev = list_eventos -> type;
		time_ev = list_eventos -> time;
		list_eventos = remover(list_eventos);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\ttime=%lf\n", type_ev, time_ev);
	}
	printf("\nlist ACTUAL\n");
	imprimir(list_eventos);

	if ( list_eventos != NULL)
	{
		type_ev = list_eventos -> type;
		time_ev = list_eventos -> time;
		list_eventos = remover(list_eventos);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\ttime=%lf\n", type_ev, time_ev);
	}
	printf("\nlist ACTUAL\n");
	imprimir(list_eventos);

	if ( list_eventos != NULL)
	{
		type_ev = list_eventos -> type;
		time_ev = list_eventos -> time;
		list_eventos = remover(list_eventos);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\ttime=%lf\n", type_ev, time_ev);
	}
	printf("\nlist ACTUAL\n");
	imprimir(list_eventos);

	if ( list_eventos != NULL)
	{
		type_ev = list_eventos -> type;
		time_ev = list_eventos -> time;
		list_eventos = remover(list_eventos);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\ttime=%lf\n", type_ev, time_ev);
	}
	printf("\nlist ACTUAL\n");
	imprimir(list_eventos);

	if ( list_eventos != NULL)
	{
		type_ev = list_eventos -> type;
		time_ev = list_eventos -> time;
		list_eventos = remover(list_eventos);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\ttime=%lf\n", type_ev, time_ev);
	}
	printf("\nlist ACTUAL\n");
	imprimir(list_eventos);

	if ( list_eventos != NULL)
	{
		type_ev = list_eventos -> type;
		time_ev = list_eventos -> time;
		list_eventos = remover(list_eventos);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\ttime=%lf\n", type_ev, time_ev);
	}
	printf("\nlist ACTUAL\n");
	imprimir(list_eventos);

	list_eventos = adicionar(list_eventos, 2, 0.5);
	list_eventos = adicionar(list_eventos, 1, 0.3);
	printf("\nlist ACTUAL\n");
	imprimir(list_eventos);
}*/
