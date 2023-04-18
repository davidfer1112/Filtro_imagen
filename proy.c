#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> // libreria para utilizar hilos
#include <unistd.h> //libreria para poder utilizar sleep


#define 	MAXPIXELS	1000;

// estructura para la imagen

typedef struct BMP
{
	char bm[2];					//(2 Bytes) BM (Tipo de archivo)
	int tamano;					//(4 Bytes) Tamaño del archivo en bytes
	int reservado;					//(4 Bytes) Reservado
	int offset;						//(4 Bytes) offset, distancia en bytes entre la img y los píxeles
	int tamanoMetadatos;			//(4 Bytes) Tamaño de Metadatos (tamaño de esta estructura = 40)
	int alto;						//(4 Bytes) Ancho (numero de píxeles horizontales)
	int ancho;					//(4 Bytes) Alto (numero de pixeles verticales)
	short int numeroPlanos;			//(2 Bytes) Numero de planos de color
	short int profundidadColor;		//(2 Bytes) Profundidad de color (debe ser 24 para nuestro caso)
	int tipoCompresion;				//(4 Bytes) Tipo de compresión (Vale 0, ya que el bmp es descomprimido)
	int tamanoEstructura;			//(4 Bytes) Tamaño de la estructura Imagen (Paleta)
	int pxmh;					//(4 Bytes) Píxeles por metro horizontal
	int pxmv;					//(4 Bytes) Píxeles por metro vertical
	int coloresUsados;				//(4 Bytes) Cantidad de colores usados 
	int coloresImportantes;			//(4 Bytes) Cantidad de colores importantes
	unsigned char ***pixel; 			//Puntero a una tabla dinamica de caracteres de 3 dimensiones para almacenar los pixeles
}BMP;


//estructura para pasar como parametro a las funciones de transformacion

typedef struct Proce{
	int limsuper;
	int liminfe;
	BMP* imagen;
	//float regiones[MAXPIXELS][MAXPIXELS];
}Proce;



//funcion de transformacion dos (hilos)
void * procesoPromedio(void *data){

	Proce* datproce = (Proce*) data; //castea y almacena los datos que le llega por parametro

	int i,j,k;

	float limsu = datproce->limsuper; // indica el limite superior de trabajo del hilo
	float limin = datproce->liminfe; // indica el limite inferior de trabajo del hilo

	BMP* imagen = datproce->imagen; // almacena la imagen a trabajar


	unsigned char temp;

   //for (i=limin;i<limsu;i++) {
		//for (j=0;j<imagen->ancho;j++) {  

		   // castea el promedio de colores de un pixel y lo almacena en un unsigned char
		  // temp = (unsigned char) (imagen->pixel[i][j][0] + imagen->pixel[i][j][1]+ imagen->pixel[i][j][2])/3;

		  //for (k=0;k<3;k++) imagen->pixel[i][j][k]= (unsigned char)temp; 	//Formula correcta
       // }  

   //}

  float im;
  
  for (i=limin;i<limsu;i++) {
		for (j=0;j<imagen->ancho;j++) {  

      
      im = (imagen->pixel[i][j][2]+imagen->pixel[i][j][1]+ imagen->pixel[i][j][0])/3;
        
		  for (k=0;k<3;k++) imagen->pixel[i][j][k]= (unsigned char)im; 	//Formula 

  
      }  

   }


	pthread_exit(NULL); //finaliza la funcion del hilo
}



//funcion de transformacion uno (hilos)
void * procesoSeparado(void *data){ 
  
  Proce* datproce = (Proce*) data;

  int i,j,k;

  unsigned char temp;
  
	float limsu = datproce->limsuper;
	float limin = datproce->liminfe;

	BMP* imagen = datproce->imagen;


  for (i=limin;i<limsu;i++) { 
	for (j=0;j<imagen->ancho;j++) {  

		// aplica el filtro de blanco y negro multiplicando valores segun el color (RGB)
		temp = (unsigned char)((imagen->pixel[i][j][2]*0.3)+(imagen->pixel[i][j][1]*0.59)+ (imagen->pixel[i][j][0]*0.11));

		for (k=0;k<3;k++) imagen->pixel[i][j][k]= (unsigned char)temp; 	//Formula correcta
    }   
  }

   // Terminar el hilo
    pthread_exit(NULL);

}



//funcion de transformacion tres (hilos)
void * procesoNuestro(void *data){ 
  
  Proce* datproce = (Proce*) data;

  int i,j,k;

  unsigned char temp;
  
	float limsu = datproce->limsuper;
	float limin = datproce->liminfe;

	BMP* imagen = datproce->imagen;


	for (i=limin; i<limsu; i++) {

    	for (j=0; j<imagen->ancho; j++) {

      		int sumaR = 0;
      		int sumaG = 0;
      		int sumaB = 0;

      		for (k=0; k<3; k++) { // suma de los colores (RGB)

        		sumaR += imagen->pixel[i][j][0];
        		sumaG += imagen->pixel[i][j][1];
        		sumaB += imagen->pixel[i][j][2];
      		}

			// Promedia la suma de los colores anteriores
      		unsigned char promedioR = (unsigned char)(sumaR / 3);
      		unsigned char promedioG = (unsigned char)(sumaG / 3);
      		unsigned char promedioB = (unsigned char)(sumaB / 3);
			  
			// Aplica la el filtro por color  
      		imagen->pixel[i][j][0] = promedioR * 1.5; // Canal rojo
      		imagen->pixel[i][j][1] = promedioG * 1.5; // Canal verde
      		imagen->pixel[i][j][2] = promedioB * 0.5; // Canal azul
			  
    	}
  }

   // Terminar el hilo
    pthread_exit(NULL);

}


void abrir_imagen(BMP *imagen, char ruta[]);
void crear_imagen(BMP *imagen, char ruta[]);	//Función para crear una imagen BMP
void convertir_imagen(BMP *imagen,int nhilos,int opci); //2 sera el numero de hilos	


// programa principal

int main (int argc, char* argv[])
{	
  int i,j,k; 				//Variables auxiliares para loops
	BMP img;				//Estructura de tipo imágen
	char IMAGEN[45];		//Almacenará la ruta de la imagen
	
   	int nhil = atoi(argv[8]); // almacena el numero de hilos que se utilizara
  
	int opci = atoi(argv[6]);

	char string[20]; //almacenará la ruta de la imagen de salida

	strcpy(string,argv[4]); //almacena la ruta de la imagen de salida

    // si el programa recibe una cantidad de difernete de parametros, le indica al usuario como 
    // de deben de ingresar esto y se sale
	if (argc!=9)
	{
        printf("\nTiene que digitar el comando se la siguiente manera: ");
        printf("\n$ %s –i nombre_imagen.bmp –t nombre_de_imagen_modificada –o opcion_de_modificacion –h numero_de_hilos",argv[0]);  
		exit(1);
	}

	// si la cantidad de hilos es menor a cero o mayor a 12, advierte al usuario y se sale
	if(nhil <= 0 || nhil > 12) {
			printf("\nEl numero de hilos no puede ser inferior o igual a 0 o mayor a 12\n");
			exit(1);
	}

	// si los hilos no son pares no advierte al usuario y se sale
	if(nhil % 2 != 0){
		printf("\nEl numero de hilos no puede ser impar\n");
		exit(1);
	}

	//Almacenar la ruta de la imágen
	strcpy(IMAGEN,argv[2]);

	abrir_imagen(&img,IMAGEN); // abre la imagen que se le paso como parametro 

	printf("\n*************************************************************************");
	printf("\nIMAGEN: %s",IMAGEN);
	printf("\n*************************************************************************");
	printf("\nDimensiones de la imágen:\tAlto=%d\tAncho=%d\n",img.alto,img.ancho);
	convertir_imagen(&img,nhil,opci); //llama la funcion de transformacion


	crear_imagen(&img,string); // crea la imagen con el nombre que le paso como paramtro el usuario
	printf("\nImágen BMP tratada en el archivo: %s \n",string);


	exit (0);	
}


//Funcion para abrir la imagen que se va a moficar
void abrir_imagen(BMP *imagen, char *ruta)
{
	FILE *archivo;	//Puntero FILE para el archivo de imágen a abrir
	int i,j,k;
        unsigned char P[3];
	
	//Abrir el archivo de imágen
	archivo = fopen( ruta, "rb+" );
	if(!archivo)
	{ 
		//Si la imágen no se encuentra en la ruta dada
		printf( "La imágen %s no se encontro\n",ruta);
		exit(1);
	}

	//Leer la cabecera de la imagen y almacenarla en la estructura a la que apunta imagen
	fseek( archivo,0, SEEK_SET);
	fread(&imagen->bm,sizeof(char),2, archivo);
	fread(&imagen->tamano,sizeof(int),1, archivo);
	fread(&imagen->reservado,sizeof(int),1, archivo);	
	fread(&imagen->offset,sizeof(int),1, archivo);	
	fread(&imagen->tamanoMetadatos,sizeof(int),1, archivo);	
	fread(&imagen->alto,sizeof(int),1, archivo);	
	fread(&imagen->ancho,sizeof(int),1, archivo);	
	fread(&imagen->numeroPlanos,sizeof(short int),1, archivo);	
	fread(&imagen->profundidadColor,sizeof(short int),1, archivo);	
	fread(&imagen->tipoCompresion,sizeof(int),1, archivo);
	fread(&imagen->tamanoEstructura,sizeof(int),1, archivo);
	fread(&imagen->pxmh,sizeof(int),1, archivo);
	fread(&imagen->pxmv,sizeof(int),1, archivo);
	fread(&imagen->coloresUsados,sizeof(int),1, archivo);
	fread(&imagen->coloresImportantes,sizeof(int),1, archivo);	

	//Validar ciertos datos de la cabecera de la imágen	
	if (imagen->bm[0]!='B'||imagen->bm[1]!='M')	
	{
		printf ("La imagen debe ser un bitmap.\n"); 
		exit(1);
	}
	if (imagen->profundidadColor!= 24) 
	{
		printf ("La imagen debe ser de 24 bits.\n"); 
		exit(1);
	}

	//Reservar memoria para la matriz de pixels 

	imagen->pixel=malloc (imagen->alto* sizeof(char *)); 
	for( i=0; i<imagen->alto; i++){
		imagen->pixel[i]=malloc (imagen->ancho* sizeof(char*));
                                     
	}

        
        for( i=0; i<imagen->alto; i++){
            for( j=0; j<imagen->ancho; j++)
	      imagen->pixel[i][j]=malloc(3*sizeof(char));
        
	}

	//Pasar la imágen a el arreglo reservado en escala de grises
	//unsigned char R,B,G;
	    
	for (i=0;i<imagen->alto;i++)
	{
		for (j=0;j<imagen->ancho;j++){  
		  for (k=0;k<3;k++) {
                      fread(&P[k],sizeof(char),1, archivo);  //Byte Blue del pixel
                      imagen->pixel[i][j][k]=(unsigned char)P[k]; 	//Formula correcta
                  }
			
		}   
	}

	
	//Cerrrar el archivo
	fclose(archivo);	
}



//Funcion para crear una nueva imgen apartir de la modificacion
void crear_imagen(BMP *imagen, char ruta[])
{
	FILE *archivo;	//Puntero FILE para el archivo de imágen a abrir

	int i,j,k;

	//Abrir el archivo de imágen
	archivo = fopen( ruta, "wb+" );
	if(!archivo)
	{ 
		//Si la imágen no se encuentra en la ruta dada
		printf( "La imágen %s no se pudo crear\n",ruta);
		exit(1);
	}
	
	//Escribir la cabecera de la imagen en el archivo
	fseek( archivo,0, SEEK_SET);
	fwrite(&imagen->bm,sizeof(char),2, archivo);
	fwrite(&imagen->tamano,sizeof(int),1, archivo);	
	fwrite(&imagen->reservado,sizeof(int),1, archivo);	
	fwrite(&imagen->offset,sizeof(int),1, archivo);	
	fwrite(&imagen->tamanoMetadatos,sizeof(int),1, archivo);	
	fwrite(&imagen->alto,sizeof(int),1, archivo);	
	fwrite(&imagen->ancho,sizeof(int),1, archivo);	
	fwrite(&imagen->numeroPlanos,sizeof(short int),1, archivo);	
	fwrite(&imagen->profundidadColor,sizeof(short int),1, archivo);	
	fwrite(&imagen->tipoCompresion,sizeof(int),1, archivo);
	fwrite(&imagen->tamanoEstructura,sizeof(int),1, archivo);
	fwrite(&imagen->pxmh,sizeof(int),1, archivo);
	fwrite(&imagen->pxmv,sizeof(int),1, archivo);
	fwrite(&imagen->coloresUsados,sizeof(int),1, archivo);
	fwrite(&imagen->coloresImportantes,sizeof(int),1, archivo);	
			
	//Pasar la imágen del arreglo reservado en escala de grises a el archivo (Deben escribirse los valores BGR)
	for (i=0;i<imagen->alto;i++)
	{
		for (j=0;j<imagen->ancho;j++)
		{  

                    for (k=0;k<3;k++)
		       fwrite(&imagen->pixel[i][j][k],sizeof(char),1, archivo);  //Escribir el Byte Blue del pixel 
		    
		    
		}   
	}
	//Cerrrar el archivo
	fclose(archivo);
}


// funcion para convertir imagen segun la opcion y cantidad de hilos
void convertir_imagen(BMP *imagen,int nhilos,int opci){

	int i,j,k;

	// switch para separar segun el la opcion elegida
	switch(opci){

		case 1:

			// los if clasifica segun la cantidad de hilos con los que se va a trabajar

			if(nhilos == 2){


				Proce proce[nhilos]; // se define un arreglo de tipo Proce, el cual la cantidad depende de la cantidad de hilos
				
				float aux = 0; // auxiliar que ayudara a definir los limites superiores o inferiores de los hilos

				int cont = 0; // contador que ayudara a rectificar que toda la imgen se modifico

				float inter = imagen->alto / nhilos; // se define el intervalo de los hilos

				//Ciclo que inicializa los datos de la variable de tipo Proce
				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen; // se le indica que imagen esta modificando
					proce[i].liminfe = aux; // indica cual es el limite inferior de trabajo
					aux = aux + inter;  // suma el intervalo al auxiliar
					proce[i].limsuper = aux; //asigna el limite superior de trabajo del hilo
					cont++;

				}


				//verifica que toda la imagen se ha modificado, si hay pixeles que no se modificaron 
				//Por que el intervalo no es un nunero entero y quedan los limites superiores incompletos 
				//se le asigna como limite superior el alto de la imagen al ultimo hilo
				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}


				//creacion de hilos 
				pthread_t pro1;
  				pthread_t pro2;

				//asigna la la funcion del hilo y pasamos como parametro el tipo de dato Proce
				pthread_create(&pro1, NULL, procesoSeparado, (void*) &proce[0]);
				pthread_create(&pro2, NULL, procesoSeparado, (void*) &proce[1]);

				//ejecuta los hilos para el proceso
  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);

			}
			else if(nhilos == 4){

				Proce proce[nhilos];
				
				float aux = 0;

				int cont = 0;

				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}
				
				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;

				pthread_create(&pro1, NULL, procesoSeparado, (void*) &proce[0]);
				pthread_create(&pro2, NULL, procesoSeparado, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoSeparado, (void*) &proce[2]);
				pthread_create(&pro4, NULL, procesoSeparado, (void*) &proce[3]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);

			}
			else if(nhilos == 6){

				Proce proce[nhilos];
				
				float aux = 0;

				int cont = 0;

				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;
				pthread_t pro5;
  				pthread_t pro6;

				pthread_create(&pro1, NULL, procesoSeparado, (void*) &proce[0]);
				pthread_create(&pro2, NULL, procesoSeparado, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoSeparado, (void*) &proce[2]);
				pthread_create(&pro4, NULL, procesoSeparado, (void*) &proce[3]);
				pthread_create(&pro5, NULL, procesoSeparado, (void*) &proce[4]);
				pthread_create(&pro6, NULL, procesoSeparado, (void*) &proce[5]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);
				pthread_join(pro5,NULL);
  				pthread_join(pro6,NULL);

			}
			else if(nhilos == 8){

				Proce proce[nhilos];
				
				float aux = 0;

				int cont = 0;

				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;
				pthread_t pro5;
  				pthread_t pro6;
				pthread_t pro7;
  				pthread_t pro8;

				pthread_create(&pro1, NULL, procesoSeparado, (void*) &proce[0]);
				pthread_create(&pro2, NULL, procesoSeparado, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoSeparado, (void*) &proce[2]);
				pthread_create(&pro4, NULL, procesoSeparado, (void*) &proce[3]);
				pthread_create(&pro5, NULL, procesoSeparado, (void*) &proce[4]);
				pthread_create(&pro6, NULL, procesoSeparado, (void*) &proce[5]);
				pthread_create(&pro7, NULL, procesoSeparado, (void*) &proce[6]);
				pthread_create(&pro8, NULL, procesoSeparado, (void*) &proce[7]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);
				pthread_join(pro5,NULL);
  				pthread_join(pro6,NULL);
				pthread_join(pro7,NULL);
  				pthread_join(pro8,NULL);

			}
			else if(nhilos == 10){

				Proce proce[nhilos];
				
				float aux = 0;

				int cont = 0;

				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;
				pthread_t pro5;
  				pthread_t pro6;
				pthread_t pro7;
  				pthread_t pro8;
				pthread_t pro9;
  				pthread_t pro10;

				pthread_create(&pro1, NULL, procesoSeparado, (void*) &proce[0]);
				pthread_create(&pro2, NULL, procesoSeparado, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoSeparado, (void*) &proce[2]);
				pthread_create(&pro4, NULL, procesoSeparado, (void*) &proce[3]);
				pthread_create(&pro5, NULL, procesoSeparado, (void*) &proce[4]);
				pthread_create(&pro6, NULL, procesoSeparado, (void*) &proce[5]);
				pthread_create(&pro7, NULL, procesoSeparado, (void*) &proce[6]);
				pthread_create(&pro8, NULL, procesoSeparado, (void*) &proce[7]);
				pthread_create(&pro9, NULL, procesoSeparado, (void*) &proce[8]);
				pthread_create(&pro10, NULL, procesoSeparado, (void*) &proce[9]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);
				pthread_join(pro5,NULL);
  				pthread_join(pro6,NULL);
				pthread_join(pro7,NULL);
  				pthread_join(pro8,NULL);
				pthread_join(pro9,NULL);
  				pthread_join(pro10,NULL);

			}
			else if(nhilos == 12){

				Proce proce[nhilos];
				
				float aux = 0;

				int cont = 0;

				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;
				pthread_t pro5;
  				pthread_t pro6;
				pthread_t pro7;
  				pthread_t pro8;
				pthread_t pro9;
  				pthread_t pro10;
				pthread_t pro11;
  				pthread_t pro12;

				pthread_create(&pro1, NULL, procesoSeparado, (void*) &proce[0]);
				pthread_create(&pro2, NULL, procesoSeparado, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoSeparado, (void*) &proce[2]);
				pthread_create(&pro4, NULL, procesoSeparado, (void*) &proce[3]);
				pthread_create(&pro5, NULL, procesoSeparado, (void*) &proce[4]);
				pthread_create(&pro6, NULL, procesoSeparado, (void*) &proce[5]);
				pthread_create(&pro7, NULL, procesoSeparado, (void*) &proce[6]);
				pthread_create(&pro8, NULL, procesoSeparado, (void*) &proce[7]);
				pthread_create(&pro9, NULL, procesoSeparado, (void*) &proce[8]);
				pthread_create(&pro10, NULL, procesoSeparado, (void*) &proce[9]);
				pthread_create(&pro11, NULL, procesoSeparado, (void*) &proce[10]);
				pthread_create(&pro12, NULL, procesoSeparado, (void*) &proce[11]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);
				pthread_join(pro5,NULL);
  				pthread_join(pro6,NULL);
				pthread_join(pro7,NULL);
  				pthread_join(pro8,NULL);
				pthread_join(pro9,NULL);
  				pthread_join(pro10,NULL);
				pthread_join(pro11,NULL);
  				pthread_join(pro12,NULL);
			}



		break;


		case 2:


			if(nhilos == 2){

				Proce proce[nhilos];
				float aux = 0;
				int cont = 0;
				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;

				pthread_create(&pro1, NULL, procesoPromedio, (void*) &proce[0]); // Enviar la matriz con los valores promedio
				pthread_create(&pro2, NULL, procesoPromedio, (void*) &proce[1]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);

			}else if(nhilos == 4){
				
				Proce proce[nhilos];
				float aux = 0;
				int cont = 0;
				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}


				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;

				pthread_create(&pro1, NULL, procesoPromedio, (void*) &proce[0]); // Enviar la matriz con los valores promedio
				pthread_create(&pro2, NULL, procesoPromedio, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoPromedio, (void*) &proce[2]); 
				pthread_create(&pro4, NULL, procesoPromedio, (void*) &proce[3]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);


			}else if(nhilos == 6){

				Proce proce[nhilos];
				float aux = 0;
				int cont = 0;
				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;
				pthread_t pro5;
  				pthread_t pro6;

				pthread_create(&pro1, NULL, procesoPromedio, (void*) &proce[0]); // Enviar la matriz con los valores promedio
				pthread_create(&pro2, NULL, procesoPromedio, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoPromedio, (void*) &proce[2]); 
				pthread_create(&pro4, NULL, procesoPromedio, (void*) &proce[3]);
				pthread_create(&pro5, NULL, procesoPromedio, (void*) &proce[4]); 
				pthread_create(&pro6, NULL, procesoPromedio, (void*) &proce[5]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);
				pthread_join(pro5,NULL);
  				pthread_join(pro6,NULL);

			}else if(nhilos == 8){


				Proce proce[nhilos];
				float aux = 0;
				int cont = 0;
				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;
				pthread_t pro5;
  				pthread_t pro6;
				pthread_t pro7;
  				pthread_t pro8;

				pthread_create(&pro1, NULL, procesoPromedio, (void*) &proce[0]); // Enviar la matriz con los valores promedio
				pthread_create(&pro2, NULL, procesoPromedio, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoPromedio, (void*) &proce[2]); 
				pthread_create(&pro4, NULL, procesoPromedio, (void*) &proce[3]);
				pthread_create(&pro5, NULL, procesoPromedio, (void*) &proce[4]); 
				pthread_create(&pro6, NULL, procesoPromedio, (void*) &proce[5]);
				pthread_create(&pro7, NULL, procesoPromedio, (void*) &proce[6]); 
				pthread_create(&pro8, NULL, procesoPromedio, (void*) &proce[7]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);
				pthread_join(pro5,NULL);
  				pthread_join(pro6,NULL);
				pthread_join(pro7,NULL);
  				pthread_join(pro8,NULL);

			}else if(nhilos == 10){

				Proce proce[nhilos];
				float aux = 0;
				int cont = 0;
				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;
				pthread_t pro5;
  				pthread_t pro6;
				pthread_t pro7;
  				pthread_t pro8;
				pthread_t pro9;
  				pthread_t pro10;

				pthread_create(&pro1, NULL, procesoPromedio, (void*) &proce[0]); // Enviar la matriz con los valores promedio
				pthread_create(&pro2, NULL, procesoPromedio, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoPromedio, (void*) &proce[2]); 
				pthread_create(&pro4, NULL, procesoPromedio, (void*) &proce[3]);
				pthread_create(&pro5, NULL, procesoPromedio, (void*) &proce[4]); 
				pthread_create(&pro6, NULL, procesoPromedio, (void*) &proce[5]);
				pthread_create(&pro7, NULL, procesoPromedio, (void*) &proce[6]); 
				pthread_create(&pro8, NULL, procesoPromedio, (void*) &proce[7]);
				pthread_create(&pro9, NULL, procesoPromedio, (void*) &proce[8]); 
				pthread_create(&pro10, NULL, procesoPromedio, (void*) &proce[9]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);
				pthread_join(pro5,NULL);
  				pthread_join(pro6,NULL);
				pthread_join(pro7,NULL);
  				pthread_join(pro8,NULL);
				pthread_join(pro9,NULL);
  				pthread_join(pro10,NULL);


			}else if(nhilos == 12){

				Proce proce[nhilos];
				float aux = 0;
				int cont = 0;
				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;
				pthread_t pro5;
  				pthread_t pro6;
				pthread_t pro7;
  				pthread_t pro8;
				pthread_t pro9;
  				pthread_t pro10;
				pthread_t pro11;
  				pthread_t pro12;

				pthread_create(&pro1, NULL, procesoPromedio, (void*) &proce[0]); // Enviar la matriz con los valores promedio
				pthread_create(&pro2, NULL, procesoPromedio, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoPromedio, (void*) &proce[2]); 
				pthread_create(&pro4, NULL, procesoPromedio, (void*) &proce[3]);
				pthread_create(&pro5, NULL, procesoPromedio, (void*) &proce[4]); 
				pthread_create(&pro6, NULL, procesoPromedio, (void*) &proce[5]);
				pthread_create(&pro7, NULL, procesoPromedio, (void*) &proce[6]); 
				pthread_create(&pro8, NULL, procesoPromedio, (void*) &proce[7]);
				pthread_create(&pro9, NULL, procesoPromedio, (void*) &proce[8]); 
				pthread_create(&pro10, NULL, procesoPromedio, (void*) &proce[9]);
				pthread_create(&pro11, NULL, procesoPromedio, (void*) &proce[10]); 
				pthread_create(&pro12, NULL, procesoPromedio, (void*) &proce[11]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);
				pthread_join(pro5,NULL);
  				pthread_join(pro6,NULL);
				pthread_join(pro7,NULL);
  				pthread_join(pro8,NULL);
				pthread_join(pro9,NULL);
  				pthread_join(pro10,NULL);
				pthread_join(pro11,NULL);
  				pthread_join(pro12,NULL);


			}



   	 	break;


		case 3:

			if(nhilos == 2){

				Proce proce[nhilos];
				float aux = 0;
				int cont = 0;
				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;

				pthread_create(&pro1, NULL, procesoNuestro, (void*) &proce[0]); // Enviar la matriz con los valores promedio
				pthread_create(&pro2, NULL, procesoNuestro, (void*) &proce[1]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);

			}else if(nhilos == 4){
				
				Proce proce[nhilos];
				float aux = 0;
				int cont = 0;
				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}


				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;

				pthread_create(&pro1, NULL, procesoNuestro, (void*) &proce[0]); // Enviar la matriz con los valores promedio
				pthread_create(&pro2, NULL, procesoNuestro, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoNuestro, (void*) &proce[2]); 
				pthread_create(&pro4, NULL, procesoNuestro, (void*) &proce[3]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);


			}else if(nhilos == 6){

				Proce proce[nhilos];
				float aux = 0;
				int cont = 0;
				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;
				pthread_t pro5;
  				pthread_t pro6;

				pthread_create(&pro1, NULL, procesoNuestro, (void*) &proce[0]); // Enviar la matriz con los valores promedio
				pthread_create(&pro2, NULL, procesoNuestro, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoNuestro, (void*) &proce[2]); 
				pthread_create(&pro4, NULL, procesoNuestro, (void*) &proce[3]);
				pthread_create(&pro5, NULL, procesoNuestro, (void*) &proce[4]); 
				pthread_create(&pro6, NULL, procesoNuestro, (void*) &proce[5]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);
				pthread_join(pro5,NULL);
  				pthread_join(pro6,NULL);

			}else if(nhilos == 8){


				Proce proce[nhilos];
				float aux = 0;
				int cont = 0;
				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;
				pthread_t pro5;
  				pthread_t pro6;
				pthread_t pro7;
  				pthread_t pro8;

				pthread_create(&pro1, NULL, procesoNuestro, (void*) &proce[0]); // Enviar la matriz con los valores promedio
				pthread_create(&pro2, NULL, procesoNuestro, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoNuestro, (void*) &proce[2]); 
				pthread_create(&pro4, NULL, procesoNuestro, (void*) &proce[3]);
				pthread_create(&pro5, NULL, procesoNuestro, (void*) &proce[4]); 
				pthread_create(&pro6, NULL, procesoNuestro, (void*) &proce[5]);
				pthread_create(&pro7, NULL, procesoNuestro, (void*) &proce[6]); 
				pthread_create(&pro8, NULL, procesoNuestro, (void*) &proce[7]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);
				pthread_join(pro5,NULL);
  				pthread_join(pro6,NULL);
				pthread_join(pro7,NULL);
  				pthread_join(pro8,NULL);

			}else if(nhilos == 10){

				Proce proce[nhilos];
				float aux = 0;
				int cont = 0;
				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;
				pthread_t pro5;
  				pthread_t pro6;
				pthread_t pro7;
  				pthread_t pro8;
				pthread_t pro9;
  				pthread_t pro10;

				pthread_create(&pro1, NULL, procesoNuestro, (void*) &proce[0]); // Enviar la matriz con los valores promedio
				pthread_create(&pro2, NULL, procesoNuestro, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoNuestro, (void*) &proce[2]); 
				pthread_create(&pro4, NULL, procesoNuestro, (void*) &proce[3]);
				pthread_create(&pro5, NULL, procesoNuestro, (void*) &proce[4]); 
				pthread_create(&pro6, NULL, procesoNuestro, (void*) &proce[5]);
				pthread_create(&pro7, NULL, procesoNuestro, (void*) &proce[6]); 
				pthread_create(&pro8, NULL, procesoNuestro, (void*) &proce[7]);
				pthread_create(&pro9, NULL, procesoNuestro, (void*) &proce[8]); 
				pthread_create(&pro10, NULL, procesoNuestro, (void*) &proce[9]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);
				pthread_join(pro5,NULL);
  				pthread_join(pro6,NULL);
				pthread_join(pro7,NULL);
  				pthread_join(pro8,NULL);
				pthread_join(pro9,NULL);
  				pthread_join(pro10,NULL);


			}else if(nhilos == 12){

				Proce proce[nhilos];
				float aux = 0;
				int cont = 0;
				float inter = imagen->alto / nhilos;

				for(i=0;i<nhilos;i++){

					proce[i].imagen = imagen;
					proce[i].liminfe = aux;
					aux = aux + inter;
					proce[i].limsuper = aux;
					cont++;

				}

				if( (imagen->alto - proce[cont-1].limsuper)  > 0 ){
					proce[cont-1].limsuper = imagen->alto;
				}

				pthread_t pro1;
  				pthread_t pro2;
				pthread_t pro3;
  				pthread_t pro4;
				pthread_t pro5;
  				pthread_t pro6;
				pthread_t pro7;
  				pthread_t pro8;
				pthread_t pro9;
  				pthread_t pro10;
				pthread_t pro11;
  				pthread_t pro12;

				pthread_create(&pro1, NULL, procesoNuestro, (void*) &proce[0]); // Enviar la matriz con los valores promedio
				pthread_create(&pro2, NULL, procesoNuestro, (void*) &proce[1]);
				pthread_create(&pro3, NULL, procesoNuestro, (void*) &proce[2]); 
				pthread_create(&pro4, NULL, procesoNuestro, (void*) &proce[3]);
				pthread_create(&pro5, NULL, procesoNuestro, (void*) &proce[4]); 
				pthread_create(&pro6, NULL, procesoNuestro, (void*) &proce[5]);
				pthread_create(&pro7, NULL, procesoNuestro, (void*) &proce[6]); 
				pthread_create(&pro8, NULL, procesoNuestro, (void*) &proce[7]);
				pthread_create(&pro9, NULL, procesoNuestro, (void*) &proce[8]); 
				pthread_create(&pro10, NULL, procesoNuestro, (void*) &proce[9]);
				pthread_create(&pro11, NULL, procesoNuestro, (void*) &proce[10]); 
				pthread_create(&pro12, NULL, procesoNuestro, (void*) &proce[11]);

  				pthread_join(pro1,NULL);
  				pthread_join(pro2,NULL);
				pthread_join(pro3,NULL);
  				pthread_join(pro4,NULL);
				pthread_join(pro5,NULL);
  				pthread_join(pro6,NULL);
				pthread_join(pro7,NULL);
  				pthread_join(pro8,NULL);
				pthread_join(pro9,NULL);
  				pthread_join(pro10,NULL);
				pthread_join(pro11,NULL);
  				pthread_join(pro12,NULL);


			}
			

		break;


		// si ingreasa una opcion difente le dice al usuario cuales son las opciones correctas y se sale
		default:
			printf("\nLa opcion dijitada no se encuentra");
			printf("\nLas opciones son: \n");
			printf("\n1. transformacion blanco y negro, 2. transformacion promedio, 3. transformacion de nosotros /n");

			exit(1);
		break;
	}
  
  
}

//forma de ejecicion programa (replit)

// gcc -Wno-unused-result proy.c -o archi
// ./archi -i tigre1.bmp -t tigre.bmp -o 3 -h 2