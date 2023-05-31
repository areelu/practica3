// Barber
// by vdelaluz@enesmorelia.unam.mx 
// GNU/GPL License
// 20230512

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "myvar.h"

double f(double x){
  return x*x;
}


int main(int argn, char **argc){
  int miproc, numproc;   //rango del proceso actual, número de procesos 
  MPI_Status status; //
  //int data;
  double a,b,dx, F;
  MPI_myvar range;
  int segments;
  int n_partition;
  double block;
  double sum=0.0;
  
  
  MPI_Init(&argn, &argc); /* Inicializar MPI */
  MPI_Comm_rank(MPI_COMM_WORLD,&miproc); /* Determinar el rango del proceso invocado*/
  MPI_Comm_size(MPI_COMM_WORLD,&numproc); /* Determinar el numero de procesos */
  MPI_Barrier (MPI_COMM_WORLD);


  if (miproc == 0){ //master    
    if (argn < 4){
      printf("Faltan parametros [a] [b] [range] [n_partitions]\n");
      printf("   dx = (b-a)/n*range \n");
      return 0;
    }
    if (sscanf(argc[1],"%lf",&a) != 1){
      printf("Error al convertir a.\n");
      return 0;
    }
    if(sscanf(argc[2],"%lf",&b) != 1){
      printf("Error al convertir b.\n");
      return 0;
    }
    if(sscanf(argc[3],"%i",&segments)!=1){
      printf("Error al convertir segments\n");
      return 0;
    }
    if(sscanf(argc[4],"%i",&n_partition)!=1){
      printf("Error al convertir partition\n");
      return 0;
    }
    dx = (b-a)/((double)(segments*n_partition));///tamaño de paso de las n_partitions
    block = (b-a)/((double)segments);	  
    printf("[%lf, %lf] range=%i n=%i, dx=%lf\n",a,b,segments,n_partition,dx);
  } //master reading command line

  MPI_Barrier (MPI_COMM_WORLD);
    
  
  if (miproc != 0) { // slaves
    //int flag_start = 1;
    // double F ; //result
    //data = 1;
    //F = 0.0;
    
    while(1){
       MPI_Send(&range, sizeof(range), MPI_CHARACTER, 0, 0, MPI_COMM_WORLD);
      MPI_Recv(&range, sizeof(range), MPI_CHARACTER, 0, 0, MPI_COMM_WORLD, &status);
      
      //integral de Riemman

      double x;
      int i=0;
      F=0.0;
      do{
	x = range.a+(((double)i)*range.dx);
	F += f(x)*range.dx;
	i++;
      }while(x <= range.b);

      range.F=F;
      
             //Parallel processing
    }

  }else{ //Master
    int flag = -1;
    MPI_Request request;
    double sum;
    int n=0;
    
    while (1) {
      if(flag == -1){
	MPI_Irecv(&range, sizeof(range), MPI_CHARACTER, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request); 
       flag = 1;//flag valía 0
      	
      }
            
      MPI_Test(&request, &flag, &status);
      if (flag ==1) {
	if (status.MPI_SOURCE == 1){
	  // sending information
	  // segmentar la informacion para enviarla al nodo disponible
	  // printf("nose %lf\n",range.F);
	  if (n != 0){
	    printf("Segment:[%lf,%lf] dx=%lf F=%lf\n",range.a,range.b,range.dx,range.F);}

	  sum += range.F;
	  // printf("aaaa %le asdfghj %le",sum,range.F);
	  range.a = a + n*block;
	  range.b = a + (n+1)*block;
	  range.dx = dx;
	  //range.F = range.F;
	  n++;
	  MPI_Send(&range, sizeof(range), MPI_CHARACTER, status.MPI_SOURCE, 0, MPI_COMM_WORLD); 	
	}
      
	flag = -1;
	
      }

      //stop condition
      //if ((a+n*dx) >= b){
      if (n >segments){
	printf("Número de segmentos:%i, F=%le\n",n-1,sum);
	break;
      }
    } //while(1)      
  }//Master

   MPI_Abort(MPI_COMM_WORLD,MPI_SUCCESS);

  MPI_Finalize ();
  return 0;
}
