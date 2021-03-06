#include <fstream>
#include "Oddysey_server.h"
#include "DataBase/Userdata.h"
#include "List.h"
#include "base64.h"
#include "pugixml.hpp"
#include "DataBase/Query.h"

/* Pasos para la ejecucion del servidor:
 * 1. Abrir una terminal
 * 2. Cambiar el directorio con: cd C-
 * 3. Escribir el comando: g++  -pthread  base64.cpp Chunk.cpp pugixml.cpp -std=c++11  Oddysey_server.cpp -o server


a-ljsoncpp -std=c++11
 * 4. Ingresar ./server
*/

void *manejador_conexion(void *);

class Base64;
Query* consulta= new Query();
List<string> Chunks();
long tam();
pugi::xml_document XML(int codigo);

void enviarChunks();

int socket_desc , client_sock , c;
struct sockaddr_in server , client;

int main() {
    Server *server = new Server;
    server->crear();
}


int Server::crear() {
    //Crea el socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("No se pudo crear el socket");
    }
    puts("\n\n                           ********SERVER********                 ");
    puts("\n\nEscuchando en el puerto 8888\n ");

    //Prepara el socket(esto es definido)
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);
    //Enlazar el servidor
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("Enlace fallido");
        return 1;
    }


    //Escuhar si llegan conexiones
    listen(socket_desc , 3);
    //Se aceptan y  esperan las conexiones
    puts("Esperando por conexiones...");
    c = sizeof(struct sockaddr_in);
    pthread_t thread_id;

    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Conexion aceptada");

        //Creacion del hilo para mas de un cliente
        if( pthread_create( &thread_id , NULL ,  manejador_conexion, (void*) &client_sock) < 0)
        {
            perror("No se pudo crear el thread");
            return 1;
        }

        //pthread_join( thread_id , NULL); // Esta es la base del Thread
    }

    if (client_sock < 0)
    {
        perror("Fallo aceptado");
        return 1;
    }
}







using namespace std;
void *manejador_conexion(void *socket_desc) {
    //Se obtiene el descriptor del socket
    int sock = *(int *) socket_desc;
    int read_size;
    char client_message[200];

    //Recibir mensajes del cliente
    while ((read_size = recv(sock, client_message, 300, 0)) > 0) {

        client_message[read_size] = '\0';

        //Mensaje recibido por el cliente
        cout << "Recibido " << client_message << endl;

        pugi::xml_document doc2;
        pugi::xml_parse_result result = doc2.load_string(client_message);

        cout << "Estado" << result.description();

        pugi::xml_node nodo= doc2.child("comunicacion");

        string cd = nodo.child("codigo").text().get();
        int codigo = atoi(cd.c_str());


        string ch = nodo.child("chunk").text().get();
        int chunk = atoi(ch.c_str());

        std::stringstream s;
        doc2.save(s, "");
        cout << "El XML de respuesta es: \n" << s.str() << std::endl;

        if (codigo == 0) {
            pugi::xml_document doc;
            doc = XML(chunk);


            std::stringstream ss;
            doc.save(ss, "");
            //cout << "El XML es: \n" << ss.str() << std::endl;

            string x = ss.str(); //+ "}";
            send(sock, x.c_str(), x.size(), 0);


        }else if(codigo== 1){
            Userdata user;
            user.setPassword(nodo.child("contrasena").text().get());
            user.setUsername(nodo.child("usuario").text().get());
            user.setNombre(nodo.child("nombre").text().get());
            string amigos= nodo.child("amigos").text().get();
            string favoritos= nodo.child("favoritos").text().get();

            SimpleLinkedList<string> *ListaAmigos= new SimpleLinkedList<string>();
            SimpleLinkedList<string> *ListaFav = new SimpleLinkedList<string>();

            //ListaAmigos = dividir(amigos);
            //ListaChunks= dividir(chunks);

            consulta->addNewUser(user);
        }else if(codigo==2){
            string usuario = nodo.child("usuario").text().get();
            if(consulta->Buscar(usuario)){

            }



        } else {
            send(sock, "Instrucción invalida", 20, 0);

        }
    }



        // Si se desconecta el cliente
        if (read_size == 0) {
            puts("Cliente desconectado");
           fflush(stdout);


        } else if (read_size == -1) {
            perror("Fallo");
        }

}


    List<string> Chunks() {

        List<string> *ListaChunks = new List<string>();//Lista de chunks
        FILE *fichero, *fichDest; //ficheros para pobar


        char nombreDest[80] = "ejemplo.mp3", nombreDest1[80], partes[80]; //ficheros
        long longitud;// Tamaño del fichero
        long cantidad; //bytes leidos
        int i, num_partes = 20;


        //Acceder a el fichero de origen
        char nombreOrg[] = "/home/cris/CLionProjects/P2_repositorio-master/mp3/ejemplo.mp3";
        if ((fichero = fopen(nombreOrg, "rb")) == NULL) {
            printf("No existe el fichero\n");
            exit(1);

        }

        if ((fichDest = fopen(nombreDest, "wb")) == NULL) {
            printf("No se ha podido crear el fichero destino!\n");
            exit(2);
        }
        /* Se clacula la longitud del archivo */
        fseek(fichero, 0, SEEK_END);
        longitud = ftell(fichero);
        fseek(fichero, 0, SEEK_SET);
        if ((fichDest = fopen(nombreDest, "wb")) == NULL) {
            printf("No se ha podido crear el fichero de destino\n");
            exit(2);
        }

        //Se nombra el subarchivo
        for (i = 0; i < num_partes; i++) {
            sprintf(partes, "00%i_", i);
            strcat(partes, nombreDest1);
            if ((fichDest = fopen(partes, "wb")) == NULL) {
                printf("No se ha podido crear el fichero de destino\n");
                exit(4);
            }

            unsigned char buffer[longitud]; //buffer para chunks

            fseek(fichero, i * longitud / num_partes, SEEK_SET);
            cantidad = fread(buffer, 1, longitud / num_partes, fichero);
            /*Se guarda cada parte*/
            string str = (const char *) buffer;
            string a = base64_encode(buffer, cantidad);


            //cout << "Codificado es: "<< a<< "\n\n";
            fwrite(buffer, 1, cantidad, fichDest);

            if (cantidad != longitud / num_partes) {
                printf("No se han generado todos los archivos\n");
            }

            strcpy(nombreDest1, nombreDest);
            ListaChunks->Insert(a);

        }
        //Cerrar los ficheros
        fclose(fichero);
        fclose(fichDest);


        return *ListaChunks;

    }
/*long tam(){
    char nombreOrg[]= "/home/kimberlyc/Música/ejemplo.mp3";
    FILE *fichero;
    fichero = fopen(nombreOrg, "rb");
    fseek(fichero, 0, SEEK_END);
    long longitud = ftell(fichero);
    fseek(fichero, 0, SEEK_SET);
    return longitud;

}
*/



    pugi::xml_document XML(int chunk) {
        // codigo leido del xml en el cliente
        pugi::xml_document doc;
        List<string> ListaChunks = Chunks();

        //cout << "El tamaño de la lista es " << ListaChunks.length()<<"\n";


            pugi::xml_node node = doc.append_child("comunicacion");
            pugi::xml_node descr0 = node.append_child("codigo");
            descr0.append_child(pugi::node_pcdata).set_value("00");

            std::string ss = std::to_string(chunk); //Aqui va el numero de chunk pedido
            char const *num = ss.c_str();
            std::string s = std::to_string(ListaChunks.length());
            char const *cantChunks = s.c_str();


            pugi::xml_node descr3 = node.append_child("offset");
            descr3.append_child(pugi::node_pcdata).set_value(num);
            pugi::xml_node descr4 = node.append_child("limite");
            descr4.append_child(pugi::node_pcdata).set_value(cantChunks);

            pugi::xml_node descr5 = node.append_child("Data");
            descr5.append_child(pugi::node_pcdata).set_value("Para el Json");

            std::ostringstream oss;
            oss << ListaChunks.Get(chunk);
            std::string buffer = oss.str();

            char const *buff = buffer.c_str();

            pugi::xml_node descr6 = node.append_child("mBytes");
            descr6.append_child(pugi::node_pcdata).set_value(buff);

            pugi::xml_node param = node.insert_child_after("param", descr5);


            //long tamanio = tam();

            param.append_attribute("nombre") = "Sutra";
            //param.append_attribute("tamaño") = tamanio;
            param.insert_attribute_after("path", param.attribute("nombre")) = "/home/kimberlyc/Música/ejemplo.mp3";


        return doc;
    }








