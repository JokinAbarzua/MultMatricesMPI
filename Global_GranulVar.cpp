#include <iostream>
#include <stdlib.h>
/*#undef SEEK_SET
#undef SEEK_END
#undef SEEK_CUR*/
#include <mpi.h>
using namespace std;
#define PREPARADO 1
#define OCUPADO 0
#define DORMIR 2
#define GRANULARIDAD 100

int main(int argc, char *argv[]) {
	
	MPI_Init(&argc,&argv);
	int myId;
	MPI_Comm_rank(MPI_COMM_WORLD,&myId);
	srand(33);
	
	if(myId == 0){  //master
		double acumTime = 0, tPreCom, tPostCom, tTotal;
		int nprocesos;
		MPI_Comm_size(MPI_COMM_WORLD,&nprocesos);
		int matrix1_filas, matrix1_columnas,matrix2_filas,matrix2_columnas;
		int** matrix1,** matrix2;
		
		cout << "¿Desea ingresar las matrices a mano? si: 1 / no: 0 " << endl;
		int rta;
		cin >> rta;
		
		if(rta){
			
			//Ingresar matrix 1
			cout << "Ingrese el numero de filas de la matrix A \n";
			cin >> matrix1_filas;
			cout << "Ingrese el numero de columnas de la matrix A \n";
			cin >> matrix1_columnas;
			
			matrix1 = (int**)malloc(sizeof(int*)*matrix1_filas);
			for(int i = 0; i<matrix1_filas; i++){ 
				matrix1[i] = (int*)malloc(sizeof(int)*(matrix1_columnas));
			}
			
			for (int i=0; i<matrix1_filas; i++){
				for (int j=0; j<matrix1_columnas; j++){
					cout << "Ingrese un valor para la fila " << i+1 << " y columna " << j+1 << endl;
					cin >> matrix1[i][j];
				}
			}
			
			//Ingresar matrix 2
			
			cout << "El numero de filas de la matrix B es "<< matrix1_columnas << " porque si no no puedo multiplicarla con A\n";
			matrix2_filas = matrix1_columnas;
			cout << "Ingrese el numero de columnas de la matrix B \n";
			cin >> matrix2_columnas;
			
			matrix2 = (int**)malloc(sizeof(int*)*matrix2_filas);
			for(int i=0; i<matrix2_filas;i++){
				matrix2[i] = (int*)malloc(sizeof(int)*matrix2_columnas);
			}
			
			for (int i=0; i<matrix2_filas; i++){
				for (int j=0; j<matrix2_columnas; j++){
					cout << "Ingrese un valor para la fila " << i+1 << " y columna " << j+1 << "\n";
					cin >> matrix2[i][j];
				}
				
			}
			
		}else{
			cout << "Ingrese el numero de filas de la matrix A \n";
			cin >> matrix1_filas;
			cout << "Ingrese el numero de columnas de la matrix A \n";
			cin >> matrix1_columnas;
			cout << "El numero de filas de la matrix B es "<< matrix1_columnas << " porque si no no puedo multiplicarla con A\n";
			matrix2_filas = matrix1_columnas;
			cout << "Ingrese el numero de columnas de la matrix B \n";
			cin >> matrix2_columnas;
			matrix1 = (int**)malloc(sizeof(int*)*matrix1_filas);
			for(int i = 0; i<matrix1_filas; i++){ 
				matrix1[i] = (int*)malloc(sizeof(int)*(matrix1_columnas));
			}
			for (int i=0; i<matrix1_filas; i++){
				for (int j=0; j<matrix1_columnas; j++){
					matrix1[i][j]= rand()%100;
				}
			}
			matrix2 = (int**)malloc(sizeof(int*)*matrix2_filas);
			for(int i=0; i<matrix2_filas;i++){
				matrix2[i] = (int*)malloc(sizeof(int)*matrix2_columnas);
			}
			for (int i=0; i<matrix2_filas; i++){
				for (int j=0; j<matrix2_columnas; j++){
					matrix2[i][j]= rand()%100;
				}
			}
			
			cout << "MATRICES CARGADAS:\n------------Matriz 1---------------\n";
			for(int i=0;i<matrix1_filas;i++){
			for(int j=0;j<matrix1_columnas;j++){
			cout << "[" << i << "]" << "[" << j << "] = " << matrix1[i][j] << " | ";
			}
			cout << endl;
			}
			cout << "------------Matriz 2---------------\n";
			for(int i=0;i<matrix2_filas;i++){
			for(int j=0;j<matrix2_columnas;j++){
			cout << "[" << i << "]" << "[" << j << "] = " << matrix2[i][j] << " | ";
			}
			cout << endl;
			}
			
			cout << "Imprimo  las matrices\n";
		}
		
		tTotal = MPI_Wtime(); 
		
		int tamanioBufferRecv = matrix1_columnas+matrix2_filas+2;
		
		//Datos a enviar
		int filasEnvio =matrix1_filas*matrix2_columnas; // filas envios es lo mismo que la cantidad de operaciones necesarias
		int trabajoTtal = filasEnvio;
		int columnasEnvio = matrix1_columnas+matrix2_filas;
		
		
		int** matrixEnvio = (int**)malloc(sizeof(int*)*filasEnvio);
		for(int i = 0; i<filasEnvio; i++){ 
			matrixEnvio[i] = (int*)malloc(sizeof(int)*(columnasEnvio+2));//aggrego 2 columnas para guardar las cooerdanadas del valor a calcular
		}
		
		int** matrixResult = (int**)malloc(sizeof(int*)*matrix1_filas);
		for(int i=0; i<matrix1_filas;i++){
			matrixResult[i] = (int*)malloc(sizeof(int)*matrix2_columnas);
		}
		
		int auxFilasA=0; //Me sirve para recorrer las filas de la matrix A
		int auxFilasB =0;//Me sirve para recorrer las filas de la matrix B
		int auxColumnasB=0; //Me sirve para recorrer las columnas  de la matrix B
		
		for(int i=0; i<filasEnvio; i++){            //Esta lineas sirven para armar la matriz de envio
			for(int k=0; k<columnasEnvio+2; k++){   //El + 2 es porque quiero las dos columnas para las coordenadas  
				
				
				if(k<columnasEnvio/2){ //k siempre va a ser positivo porque es el resultado de un numero multiplicado por 2
					matrixEnvio[i][k]=matrix1[auxFilasA][k];	//Puedo usar K y no me sirve un aux porque se setea a 0 en el momento corecto mientras hace el bucle
				}
				else{
					if(k>=columnasEnvio/2 && k<columnasEnvio){
						matrixEnvio[i][k]=matrix2[auxFilasB][auxColumnasB];
						auxFilasB =auxFilasB+1; //Cambiando fila de B es como recorrer B por columnas
					}
					else{
						if(k==columnasEnvio){ //Este seria el penultimo elemento de una fila de la matrix envio, ahi voy a guardar la coordenada que me dice en cual fila de la matrix resultado corresponde el valor
							matrixEnvio[i][k]=auxFilasA; //la columna de la matrix B que utilizo es igual a la fila de la matrix resultado en lo cual voy a guardar el valor
						}
						else{//Este seria el ultimo elemento e una fila de la matrix envio, ahi voy a guardar la coordenada que me dice en cual columna de la matrix resultado corresponde el valor
							matrixEnvio[i][k]=auxColumnasB; //la fila de la matrix A que utilizo es igual a la columna de la matrix resultado en lo cual voy a guardar el valor
						}
					}
				}
			}
			//Aca termino de escribir una fila de la matrix Envio, entonces cambio fila en la matrixA y reseteo la fila de la matrix B. Resetear la fila de la matrix B seria volver arriba, para recorrerla
			//de vuelta por columna
			auxFilasA=auxFilasA+1;
			auxFilasB=0;
			
			if(auxFilasA==matrix1_filas){//Cuando auxFilasA es igual a nA (nA seria el numero de fila de A), tengo que empiezar de vuelta desde la primera fila de A y pasar a la segunda columna de B
				auxFilasA=0;
				auxColumnasB=auxColumnasB+1; //no necesito nunca resetear auxColumnasB porque cuando termino las columnas de B tambien termino el calculo
			}
		}
		
		
		
		
		
		/*cout << "MAtrix envio: " << endl;
		for(int i =0; i<filasEnvio; i++){
		for(int j=0;j<columnasEnvio+2;j++){
		cout << matrixEnvio[i][j] << " ";
		}
		cout << endl;
		}*/
		
		
		tPreCom = MPI_Wtime();
		MPI_Bcast(&tamanioBufferRecv,1,MPI_INT,0,MPI_COMM_WORLD); //se envia el tamaño del buffer de recepcion necesario para que los workers lo creen
		tPostCom = MPI_Wtime();
		acumTime += (tPostCom-tPreCom);
		
		cout << "MASTER- Envio bcast " << tamanioBufferRecv << endl;
		
		int* workers = (int*)malloc(sizeof(int)*nprocesos-1); //va a seguir el estado de los workers
		int workersActivos = nprocesos-1;
		int tengoDatos;
		MPI_Status status;
		for(int i=0;i<nprocesos-1;i++){
			workers[i] = PREPARADO;           //se inicializan a los workers en estado PREPARADO
		}
		
		while(filasEnvio >= GRANULARIDAD || workersActivos !=0){          // mientras haya trabajo por hacer O tenga workers sin DORMIR
			if(filasEnvio < GRANULARIDAD){  //si no queda trabajo grande como para mandarlo a un worker o es 0
				if(filasEnvio){
					int acum = 0;
					while(filasEnvio>0){
						for(int i=0;i<(tamanioBufferRecv-2)/2;i++){                                //se hace el calculo
							acum += (matrixEnvio[trabajoTtal-filasEnvio][i] * matrixEnvio[trabajoTtal-filasEnvio][i+((tamanioBufferRecv-2)/2)]); 
						}
						matrixResult[matrixEnvio[trabajoTtal-filasEnvio][tamanioBufferRecv-2]][matrixEnvio[trabajoTtal-filasEnvio][tamanioBufferRecv-1]] = acum;
						acum = 0;
						--filasEnvio;
					}
				}
				for(int i=0;i<nprocesos-1;i++){            //este bucle elige al primer worker en estado PREPARADO y lo manda a dormir
					if(workers[i] == PREPARADO){
						int estadoEnviar = DORMIR;
						
						cout << "MASTER- Envio a dormir a worker " << i+1 << endl;
						
						tPreCom = MPI_Wtime();
						MPI_Send(&estadoEnviar,1,MPI_INT,i+1,120,MPI_COMM_WORLD);   //manda un worker a DORMIR
						tPostCom = MPI_Wtime();
						acumTime += (tPostCom-tPreCom);
						
						--workersActivos;                                           //resta uno  a la cantidad de workers activos
						workers[i] = DORMIR;                                        //actualiza el estado de ese worker
						break;
					}
				}
			}else{      //sino, osea que queda trabajo por hacer
				for(int i=0;i<nprocesos-1;i++){         //elige al primer worker PREPARADO y le avisa que se prepare y le da trabajo
					if(workers[i] == PREPARADO){
						int estadoEnviar = PREPARADO;
						
						tPreCom = MPI_Wtime();
						MPI_Send(&estadoEnviar,1,MPI_INT,i+1,120,MPI_COMM_WORLD);  //le manda el aviso preparate al worker
						tPostCom = MPI_Wtime();
						acumTime += (tPostCom-tPreCom);
						
						cout << "MASTER- Envio senial PREPARADO a worker " << i+1 << endl;
						cout << "MASTER- Envio matriz trabajo a worker ";
						cout << i+1 << " Datos : ";
						int* arregloEnvio = (int*)malloc(sizeof(int)*tamanioBufferRecv*GRANULARIDAD);
						int cont = 0;
						for(int n =0;n<GRANULARIDAD;n++){
							for(int j=0;j<columnasEnvio+2;j++){
								cout << matrixEnvio[trabajoTtal-filasEnvio+n][j] << " ";
								arregloEnvio[cont] = matrixEnvio[trabajoTtal-filasEnvio+n][j];
								++cont;
							}
							cout << endl;
						}
						
						tPreCom = MPI_Wtime();
						MPI_Send(arregloEnvio,tamanioBufferRecv*GRANULARIDAD,MPI_INT,i+1,420,MPI_COMM_WORLD); //le envia trabajo al worker
						tPostCom = MPI_Wtime();
						acumTime += (tPostCom-tPreCom);
						filasEnvio = filasEnvio - GRANULARIDAD;           //se resta uno a la cantidad de trabajo
						workers[i] = OCUPADO;    // se pone al worker al que se le envio trabajo en estado OCUPADO
						break;
					}
				}
			}
			//esta parte se ejecuta siempre que dure el while
			MPI_Iprobe(MPI_ANY_SOURCE,320,MPI_COMM_WORLD,&tengoDatos,&status); //el master se fija si hay resultados enviados por algun worker
			
			if(tengoDatos){      //si los hay, los guarda y cambia el estado del worker que envio el resultado (pasa de ocupado a preparado)
				int resultado[GRANULARIDAD][3];  // [0] resultado  [1] fila [2] columna
				MPI_Status statusaux;
				tPreCom = MPI_Wtime();
				MPI_Recv(resultado,3*GRANULARIDAD,MPI_INT,status.MPI_SOURCE,320,MPI_COMM_WORLD,&statusaux); //recibe los resultados
				tPostCom = MPI_Wtime();
				acumTime += (tPostCom-tPreCom);
				for(int i=0;i<GRANULARIDAD;i++){
					matrixResult[resultado[i][1]][resultado[i][2]] = resultado[i][0];  //los guarda
				}
				workers[status.MPI_SOURCE-1] = PREPARADO;  //pone al worker en preparado
			}  
		}
		
		double auxTime = MPI_Wtime();
		
		MPI_Barrier(MPI_COMM_WORLD);
		
		//mostrar matriz resultante de todo el arduo trabajo
		for(int i=0;i<matrix1_filas;i++){
			for(int j=0;j<matrix2_columnas;j++){
				cout << matrixResult[i][j] << " | ";
			}
			cout << endl;
		}
		
		cout << "_________________________TIEMPOS_______________________" << endl;
		cout << "Tiempo Total: " << auxTime - tTotal << endl;
		cout << "Tiempo Total Mensajes: " << acumTime << endl;
		cout << "Tiempo Total Computo: " << (auxTime - tTotal) - acumTime << endl;
		
		free(matrix1);
		free(matrix2);
		
	}else{   //worker
		
		int estado = PREPARADO; 
		int resultado[GRANULARIDAD][3];
		int acumu = 0;   //variable para el calculo del resultado
		int tamanioBufferRecv;
		MPI_Status status;
		MPI_Bcast(&tamanioBufferRecv,1,MPI_INT,0,MPI_COMM_WORLD);  //recibimos el tamaño del buffer esperado 
		
		//cout << "Worker " << myId << "- Recibo Bcast: " << tamanioBufferRecv << endl;
		
		int* recep = (int*)malloc(sizeof(int)*tamanioBufferRecv*GRANULARIDAD);
		
		while(estado != DORMIR){   // mientras no me manden a DORMIR trabajo
			
			MPI_Recv(&estado,1,MPI_INT,0,120,MPI_COMM_WORLD,&status); //recibe el estado en el que lo pone el master
			
			//cout << "Worker " << myId << "- Mi Estado: " << estado << endl;
			
			if(estado != DORMIR){   //si no lo manda a DORMIR trabaja
				MPI_Recv(recep,tamanioBufferRecv*GRANULARIDAD,MPI_INT,0,420,MPI_COMM_WORLD,&status); //recibe los datos a trabajar
				
				cout << "Worker " << myId << "- Recibo trabajo: ";
				for(int i = 0;i<GRANULARIDAD;i++){
					for(int j=0;j<tamanioBufferRecv;j++){
						cout << recep[(i*tamanioBufferRecv)+j] << " ";
					}
					cout << endl;
				}
				
				for(int j=0;j<GRANULARIDAD;j++){
					for(int i=0;i<(tamanioBufferRecv-2)/2;i++){                                //se hace el calculo
						acumu = acumu + (recep[(j*tamanioBufferRecv)+i] * recep[(j*tamanioBufferRecv)+i+((tamanioBufferRecv-2)/2)]); 
					}
					resultado[j][0] = acumu;                                    //arma el vector resultado
					resultado[j][1] = recep[(tamanioBufferRecv) + (j*tamanioBufferRecv)-2];
					resultado[j][2] = recep[(tamanioBufferRecv) + (j*tamanioBufferRecv)-1];
					acumu=0;
				}
				MPI_Send(resultado,3*GRANULARIDAD,MPI_INT,0,320,MPI_COMM_WORLD);  //envia el resultado
				
				//cout << "Worker " << myId << "- Envio resultado " << "[" << resultado[1] << "][" << resultado[2] << "]=" << resultado[0] << endl;
				
				acumu = 0;
			}
		}
		MPI_Barrier(MPI_COMM_WORLD);
		free(recep);
	}
	MPI_Finalize();
	return 0;
}
