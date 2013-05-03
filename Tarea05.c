/*Grupo: Camilo Candia Rubinstein
 *        Madeleine Opazo Cabargas
 *        Maximiliano Meza 
 * 
 * 
 * Se uso la librería libpq-dev y el programa fue compilado en gcc.
 * El comando es: gcc -lm -o Tarea05 Tarea05.c -Wall -I/usr/include/postgresql  -lpq
 * /usr/include/postgreslq es el path al archivo de libpq-fe.h
 * ******************************************************************************************/
#include<stdio.h>
#include<libpq-fe.h>
#include<string.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<time.h>
void imprimir(PGresult *resultado)
{ 
	    int tuplas,campos,i,j;
	    tuplas=PQntuples(resultado);
		campos=PQnfields(resultado);
		for(i=0;i<tuplas;i++)
		{
			for(j=0;j<campos;j++)
			{
				printf("   %s",PQgetvalue(resultado,i,j));
			}
			printf("\n");
		}
}
void instrucciones()
{
	    printf("---------------------------------------------------------------------------\n");
		printf("Intrucciones de uso:\n\n");
		printf("Para estimar la nota de un estudiante debe ingresar:\n");
		printf("./Tarea05 -i <id_estudiante>\n");
		printf("Para estimar la nota de todos los estudiantes debe ingresar:\n");
		printf("./Tarea05 -a\n");
		printf("Para calcular el promedio y la desviacion de las asignaturas de calculo:\n");
		printf("Tarea05 -s\n");
		printf("Para mostrar datos del programa:\n");
		printf("Tarea05 -v\n");
		printf("---------------------------------------------------------------------------\n");
}
PGconn * con()
{
	PGconn *conexion=NULL;
	const char *pghost="190.100.156.5"; 
	const char *pgport="5432";
	const char *pgoptions=NULL;
	const char *pgtty=NULL;
	const char *dbname="iswdb";
	const char *login="isw";
	const char *pwd="isw";
	conexion=PQsetdbLogin(pghost,pgport,pgoptions,pgtty,dbname,login,pwd);
	if(PQstatus(conexion)==CONNECTION_BAD)
	{
		printf("Problemas con la conexion:\n");
		printf("%s\n",PQerrorMessage(conexion));
	}
	else
	{
		printf("Conexion establecida\n\n");
	}
    return conexion;
}
void estudiante_id(char* id,PGconn* conexion)
{
	int num=htonl((int)id);
	const char* valores[1]={(char*)&num};
	int tamanio[1]={sizeof(num)};
	int binary[1]={0};
	PGresult *resultado=PQexecParams(conexion,"select estudiante_id,AVG(e.nota) from asignaturas_cursadas as e join cursos as c on (c.asignatura!='CÁLCULO VECTORIAL' and c.curso_id=e.curso_id and e.estudiante_id=$1::int) group by estudiante_id",1,NULL,valores,tamanio,binary,0);
	imprimir(resultado);
}
void estudiantes_full(PGconn* conexion)
{
	FILE *archivo;
	archivo=fopen("estimaciones.csv","w");
	PGresult *resultado=PQexec(conexion,"select estudiante_id,AVG(e.nota) from asignaturas_cursadas as e join cursos as c on (c.asignatura!='CÁLCULO VECTORIAL' and c.curso_id=e.curso_id ) group by estudiante_id");
	imprimir(resultado);
	fprintf(archivo,"estudiante_id                   estimación");
	fclose(archivo);
}
void asignaturas(PGconn* conexion)
{
	PGresult *resultado=NULL;
	resultado=PQexec(conexion,"select c.asignatura,AVG(e.nota),Stddev(e.nota) from cursos as c join asignaturas_cursadas as e on (c.asignatura='CÁLCULO I' and e.curso_id=c.curso_id) group by c.asignatura");
	imprimir(resultado);
	PQclear(resultado);
	resultado=PQexec(conexion,"select c.asignatura,AVG(e.nota),Stddev(e.nota) from cursos as c join asignaturas_cursadas as e on (asignatura='CÁLCULO II' and e.curso_id=c.curso_id) group by c.asignatura");
	imprimir(resultado);
	PQclear(resultado);
	resultado=PQexec(conexion,"select c.asignatura,AVG(e.nota),Stddev(e.nota) from cursos as c join asignaturas_cursadas as e on (asignatura='CÁLCULO EN VARIAS VARIABLES' and e.curso_id=c.curso_id) group by c.asignatura");
	imprimir(resultado);
	PQclear(resultado);
	resultado=PQexec(conexion,"select c.asignatura,AVG(e.nota),Stddev(e.nota) from cursos as c join asignaturas_cursadas as e on (asignatura='CÁLCULO VECTORIAL' and e.curso_id=c.curso_id) group by c.asignatura");
	imprimir(resultado);
}
int main(int argc,char** argv)
{
	PGconn *conexion=NULL;
	PGresult *resultado=NULL;
	if(argc>3 || argc<2)
	{
		//instrucciones de uso.
		instrucciones();
	}
	else
	{
		conexion=con();
		
		if(argc==3 && !strncmp(argv[1],"-i",2))
		{
			//funcion id estudiantes
			printf("id              estimador               Cálculo vactorial\n");
			estudiante_id(argv[2],conexion);
		}
		else
		{
			if(!strncmp(argv[1],"-a",2))
			{
				//Estimacion para todos los estudiantes
				printf("estimacion para todos los estudiantes\n");
				estudiantes_full(conexion);
			}
			else
			{
				if(!strncmp(argv[1],"-s",2))
				{
					//Promedio y desviacion de todas las asignaturas de calculo.
					printf("Asignatura           Promedio                desviacion\n");
					asignaturas(conexion);
				}
				else
				{
					if(!strncmp(argv[1],"-v",2))
					{
						//fecha y hora de compilacion,fecha y hora actual,versión del programa,nombre de integrantes del grupo.
						  printf("Version: 1.0\n");
						  time_t tiempo = time(0);
                          struct tm *tlocal = localtime(&tiempo);
                          char output[128];
                          strftime(output,128,"%d/%m/%y %H:%M:%S",tlocal);
                          printf("Fecha y hora actual: %s\n",output);
                          printf("Grupo: Camilo Candia Rubinstein, Madeleine Opazo Cabargas, Maximiliano Meza\n");
					}
					else
					{
						//Ingreso de un parametro invalido.
						instrucciones();
					}
				}
			}
		}
	}
	PQclear(resultado);
	PQfinish(conexion);	
	return 0;
}
