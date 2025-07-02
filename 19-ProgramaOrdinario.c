#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include <windows.h>
#include <conio.h>
#include <ctype.h>

MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;
// Prototipos de funciones
int verificarIDExiste(const char *tabla, const char *campoID, const char *id);
// Definición de colores ISO
#define COLOR_TITULO 11     // Amarillo claro
#define COLOR_SUBTITULO 14   // Amarillo
#define COLOR_NORMAL 15      // Blanco
#define COLOR_EXITO 10       // Verde
#define COLOR_ERROR 12       // Rojo
#define COLOR_CAMPO 9        // Azul claro
#define COLOR_RESALTADO 13   // Magenta

void gotoxy(int x, int y) {
    COORD coord = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void limpiarBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void mostrarEncabezado(const char *titulo) {
    system("cls");
    setColor(COLOR_TITULO);
    gotoxy(35, 2); printf("====================================");
    gotoxy(35, 3); printf("   SISTEMA DE ALQUILER DE VEHICULOS");
    gotoxy(35, 4); printf("====================================");
    
    setColor(COLOR_SUBTITULO);
    gotoxy(35, 6); printf("%s", titulo);
    gotoxy(35, 7); printf("----------------------------");
    setColor(COLOR_NORMAL);
}

void conectarBD() {
    conn = mysql_init(NULL);
    if (conn == NULL) {
        setColor(COLOR_ERROR);
        printf("Error al iniciar MySQL\n");
        exit(1);
    }
    if (!mysql_real_connect(conn, "localhost", "root", "", "AgenciaAlquiler", 0, NULL, 0)) {
        setColor(COLOR_ERROR);
        printf("Error de conexión: %s\n", mysql_error(conn));
        exit(1);
    }
}

// Función para validar si una cadena contiene solo letras y espacios
int esSoloLetras(const char *cadena) {
    for (int i = 0; cadena[i] != '\0'; i++) {
        if (!isalpha(cadena[i]) && cadena[i] != ' ') {
            return 0;
        }
    }
    return 1;
}

// Función para validar matrícula (formato: 0000-AAA)
int esMatriculaValida(const char *matricula) {
    if (strlen(matricula) != 8) return 0;
    if (!isdigit(matricula[0]) || !isdigit(matricula[1]) || 
        !isdigit(matricula[2]) || !isdigit(matricula[3])) return 0;
    if (matricula[4] != '-') return 0;
    if (!isalpha(matricula[5]) || !isalpha(matricula[6]) || !isalpha(matricula[7])) return 0;
    return 1;
}

void agregarCliente() {
    char nombre[50], apellido[50];
    
    mostrarEncabezado("AGREGAR NUEVO CLIENTE");
    
    // Validación del nombre
    do {
        setColor(COLOR_CAMPO); gotoxy(10, 10); printf("Nombre: ");
        setColor(COLOR_NORMAL); gotoxy(18, 10); 
        fgets(nombre, sizeof(nombre), stdin);
        nombre[strcspn(nombre, "\n")] = 0;
        
        if (strlen(nombre) == 0) {
            setColor(COLOR_ERROR); gotoxy(10, 12); printf("El nombre no puede estar vacío.");
            gotoxy(10, 13); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, 12); printf("                                           ");
            gotoxy(10, 13); printf("                                           ");
        } else if (!esSoloLetras(nombre)) {
            setColor(COLOR_ERROR); gotoxy(10, 12); printf("El nombre solo debe contener letras.");
            gotoxy(10, 13); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, 12); printf("                                           ");
            gotoxy(10, 13); printf("                                           ");
        }
    } while (strlen(nombre) == 0 || !esSoloLetras(nombre));

    // Validación del apellido
    do {
        setColor(COLOR_CAMPO); gotoxy(10, 12); printf("Apellido: ");
        setColor(COLOR_NORMAL); gotoxy(20, 12); 
        fgets(apellido, sizeof(apellido), stdin);
        apellido[strcspn(apellido, "\n")] = 0;
        
        if (strlen(apellido) == 0) {
            setColor(COLOR_ERROR); gotoxy(10, 14); printf("El apellido no puede estar vacío.");
            gotoxy(10, 15); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, 14); printf("                                           ");
            gotoxy(10, 15); printf("                                           ");
        } else if (!esSoloLetras(apellido)) {
            setColor(COLOR_ERROR); gotoxy(10, 14); printf("El apellido solo debe contener letras.");
            gotoxy(10, 15); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, 14); printf("                                           ");
            gotoxy(10, 15); printf("                                           ");
        }
    } while (strlen(apellido) == 0 || !esSoloLetras(apellido));

    char query[256];
    snprintf(query, sizeof(query), "INSERT INTO Clientes (Nombre, Apellido) VALUES ('%s', '%s')", nombre, apellido);

    if (mysql_query(conn, query)) {
    setColor(COLOR_ERROR); gotoxy(10, 16); printf("Error al agregar cliente: %s", mysql_error(conn));
} else {
    setColor(COLOR_EXITO); gotoxy(10, 16); printf("Cliente agregado exitosamente con ID: %ld", mysql_insert_id(conn));
}
    setColor(COLOR_NORMAL); gotoxy(10, 18); printf("Presione cualquier tecla para continuar...");
    getch();
}

void actualizarCliente() {
    int id_cliente;
    char nombre[50], apellido[50];
    
    mostrarEncabezado("ACTUALIZAR CLIENTE");
    
    // Mostrar lista de clientes
    setColor(COLOR_SUBTITULO); gotoxy(10, 9); printf("Clientes registrados:");
    setColor(COLOR_NORMAL);
    
    if (mysql_query(conn, "SELECT ID_Clientes, Nombre, Apellido FROM Clientes ORDER BY Apellido")) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("Error al consultar clientes: %s", mysql_error(conn));
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("No hay clientes registrados.");
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    int fila = 11;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(10, fila); printf("ID: %s", row[0]);
        gotoxy(20, fila); printf("Nombre: %s %s", row[1], row[2]);
        fila++;
    }
    mysql_free_result(res);
    
    // Solicitar ID del cliente a actualizar
    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila+2); printf("ID del cliente a actualizar: ");
        setColor(COLOR_NORMAL); gotoxy(38, fila+2); 
        scanf("%d", &id_cliente);
        limpiarBuffer();
        
        if (!verificarIDExiste("Clientes", "ID_Clientes", id_cliente)) {
            setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No existe un cliente con ese ID.");
            gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila+4); printf("                                           ");
            gotoxy(10, fila+5); printf("                                           ");
        }
    } while (!verificarIDExiste("Clientes", "ID_Clientes", id_cliente));
    
    // Obtener datos actuales del cliente
    char query_actual[256];
    snprintf(query_actual, sizeof(query_actual), "SELECT Nombre, Apellido FROM Clientes WHERE ID_Clientes = %d", id_cliente);
    
    if (mysql_query(conn, query_actual)) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("Error al obtener datos del cliente: %s", mysql_error(conn));
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No se encontraron datos del cliente.");
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        if (res) mysql_free_result(res);
        return;
    }
    
    row = mysql_fetch_row(res);
    strcpy(nombre, row[0]);
    strcpy(apellido, row[1]);
    mysql_free_result(res);
    
    // Solicitar nuevos datos
    gotoxy(10, fila+4); printf("Datos actuales:");
    gotoxy(10, fila+5); printf("Nombre: %s", nombre);
    gotoxy(10, fila+6); printf("Apellido: %s", apellido);
    
    setColor(COLOR_CAMPO); gotoxy(10, fila+8); printf("Nuevo nombre (dejar vacío para no cambiar): ");
    setColor(COLOR_NORMAL); gotoxy(50, fila+8); 
    fgets(nombre, sizeof(nombre), stdin);
    nombre[strcspn(nombre, "\n")] = 0;
    
    setColor(COLOR_CAMPO); gotoxy(10, fila+9); printf("Nuevo apellido (dejar vacío para no cambiar): ");
    setColor(COLOR_NORMAL); gotoxy(53, fila+9); 
    fgets(apellido, sizeof(apellido), stdin);
    apellido[strcspn(apellido, "\n")] = 0;
    
    // Construir consulta de actualización
    char query_update[256];
    char set_clause[200] = "";
    
    if (strlen(nombre) > 0 && esSoloLetras(nombre)) {
        strcat(set_clause, "Nombre = '");
        strcat(set_clause, nombre);
        strcat(set_clause, "'");
    }
    
    if (strlen(apellido) > 0 && esSoloLetras(apellido)) {
        if (strlen(set_clause) > 0) strcat(set_clause, ", ");
        strcat(set_clause, "Apellido = '");
        strcat(set_clause, apellido);
        strcat(set_clause, "'");
    }
    
    if (strlen(set_clause) == 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila+11); printf("No se proporcionaron datos para actualizar.");
        gotoxy(10, fila+12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    snprintf(query_update, sizeof(query_update), "UPDATE Clientes SET %s WHERE ID_Clientes = %d", set_clause, id_cliente);
    
    if (mysql_query(conn, query_update)) {
        setColor(COLOR_ERROR); gotoxy(10, fila+11); printf("Error al actualizar cliente: %s", mysql_error(conn));
    } else {
        setColor(COLOR_EXITO); gotoxy(10, fila+11); printf("Cliente actualizado exitosamente.");
    }
    
    gotoxy(10, fila+13); printf("Presione cualquier tecla para continuar...");
    getch();
}

int verificarIDExiste(const char *tabla, const char *campoID, const char *id) {
    char query[256];
    
    // Si el ID es numérico (sin comillas)
    if (isdigit(id[0])) {
        snprintf(query, sizeof(query), "SELECT 1 FROM %s WHERE %s = %s", tabla, campoID, id);
    } 
    // Si el ID es alfanumérico (con comillas)
    else {
        snprintf(query, sizeof(query), "SELECT 1 FROM %s WHERE %s = '%s'", tabla, campoID, id);
    }
    
    if (mysql_query(conn, query) != 0) return 0;

    res = mysql_store_result(conn);
    int existe = (res && mysql_num_rows(res) > 0);
    if (res) mysql_free_result(res);
    return existe;
}

int fechaValida(const char *fecha) {
    if (strlen(fecha) != 10 || fecha[4] != '-' || fecha[7] != '-') return 0;
    
    // Verificar que los componentes son números
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit(fecha[i])) return 0;
    }
    
    // Validación básica de rangos
    int anio = atoi(fecha);
    int mes = atoi(fecha + 5);
    int dia = atoi(fecha + 8);
    
    if (mes < 1 || mes > 12) return 0;
    if (dia < 1 || dia > 31) return 0;
    
    return 1;
}

int mostrarCochesDisponiblesMenu(char matriculas[][11], int maxCoches) {
    mostrarEncabezado("COCHES DISPONIBLES");
    
    if (mysql_query(conn, "SELECT Matricula, Modelo, Marca, Color, Precio FROM Coche WHERE Disponible = 1")) {
        setColor(COLOR_ERROR); gotoxy(10, 10); printf("Error al consultar coches: %s", mysql_error(conn));
        gotoxy(10, 11); printf("Presione cualquier tecla para continuar...");
        getch();
        return -1;
    }
    
    res = mysql_store_result(conn);
    if (!res) return -1;

    setColor(COLOR_SUBTITULO);
    gotoxy(10, 10); printf("No.");
    gotoxy(15, 10); printf("Matrícula");
    gotoxy(25, 10); printf("Modelo");
    gotoxy(40, 10); printf("Marca");
    gotoxy(55, 10); printf("Color");
    gotoxy(65, 10); printf("Precio");
    setColor(COLOR_NORMAL);
    
    int i = 0;
    int fila = 12;
    while ((row = mysql_fetch_row(res)) != NULL && i < maxCoches) {
        gotoxy(10, fila); printf("%d.", i + 1);
        gotoxy(15, fila); printf("%s", row[0]);
        gotoxy(25, fila); printf("%s", row[1]);
        gotoxy(40, fila); printf("%s", row[2]);
        gotoxy(55, fila); printf("%s", row[3]);
        gotoxy(65, fila); printf("%s", row[4]);
        
        strncpy(matriculas[i], row[0], 10);
        matriculas[i][10] = '\0';
        i++;
        fila++;
    }
    mysql_free_result(res);

    if (i == 0) {
        setColor(COLOR_ERROR); gotoxy(10, 12); printf("No hay coches disponibles.");
        gotoxy(10, 13); printf("Presione cualquier tecla para continuar...");
        getch();
        return -1;
    }

    int opcion;
    setColor(COLOR_CAMPO); gotoxy(10, fila+1); printf("Seleccione coche (1-%d): ", i);
    setColor(COLOR_NORMAL); gotoxy(35, fila+1); 
    scanf("%d", &opcion);
    limpiarBuffer();

    if (opcion < 1 || opcion > i) {
        setColor(COLOR_ERROR); gotoxy(10, fila+3); printf("Opción inválida.");
        gotoxy(10, fila+4); printf("Presione cualquier tecla para continuar...");
        getch();
        return -1;
    }
    return opcion - 1; // índice en array
}

void hacerReserva() {
    int id_cliente, id_agencia;
    char fechaIn[11], fechaFin[11];
    char matriculas[100][11];
    
    mostrarEncabezado("NUEVA RESERVA");

    // Mostrar lista de clientes
    setColor(COLOR_SUBTITULO); gotoxy(10, 9); printf("Clientes registrados:");
    setColor(COLOR_NORMAL);
    
    if (mysql_query(conn, "SELECT ID_Clientes, Nombre, Apellido FROM Clientes ORDER BY Apellido")) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("Error al consultar clientes: %s", mysql_error(conn));
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("No hay clientes registrados.");
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    int fila = 11;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(10, fila); printf("ID: %s", row[0]);
        gotoxy(20, fila); printf("Nombre: %s %s", row[1], row[2]);
        fila++;
    }
    mysql_free_result(res);
    
    // Solicitar ID del cliente
    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila+2); printf("ID Cliente: ");
        setColor(COLOR_NORMAL); gotoxy(22, fila+2); 
        scanf("%d", &id_cliente);
        limpiarBuffer();
        
        if (!verificarIDExiste("Clientes", "ID_Clientes", id_cliente)) {
            setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No existe un cliente con ese ID.");
            gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila+4); printf("                                           ");
            gotoxy(10, fila+5); printf("                                           ");
        }
    } while (!verificarIDExiste("Clientes", "ID_Clientes", id_cliente));
    
    // Mostrar lista de agencias
    setColor(COLOR_SUBTITULO); gotoxy(10, fila+4); printf("Agencias disponibles:");
    setColor(COLOR_NORMAL);
    
    if (mysql_query(conn, "SELECT ID_Agencia, NombreA, DireccionA FROM Agencia")) {
        setColor(COLOR_ERROR); gotoxy(10, fila+6); printf("Error al consultar agencias: %s", mysql_error(conn));
        gotoxy(10, fila+7); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res) {
        setColor(COLOR_ERROR); gotoxy(10, fila+6); printf("No hay agencias registradas.");
        gotoxy(10, fila+7); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    int fila_agencias = fila+6;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(10, fila_agencias); printf("ID: %s", row[0]);
        gotoxy(20, fila_agencias); printf("Nombre: %s", row[1]);
        gotoxy(50, fila_agencias); printf("Dirección: %s", row[2]);
        fila_agencias++;
    }
    mysql_free_result(res);
    
    // Solicitar ID de la agencia
    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila_agencias+2); printf("ID Agencia: ");
        setColor(COLOR_NORMAL); gotoxy(22, fila_agencias+2); 
        scanf("%d", &id_agencia);
        limpiarBuffer();
        
        if (!verificarIDExiste("Agencia", "ID_Agencia", id_agencia)) {
            setColor(COLOR_ERROR); gotoxy(10, fila_agencias+4); printf("No existe una agencia con ese ID.");
            gotoxy(10, fila_agencias+5); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila_agencias+4); printf("                                           ");
            gotoxy(10, fila_agencias+5); printf("                                           ");
        }
    } while (!verificarIDExiste("Agencia", "ID_Agencia", id_agencia));
    
    // Solicitar fechas
    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila_agencias+4); printf("Fecha inicio (YYYY-MM-DD): ");
        setColor(COLOR_NORMAL); gotoxy(35, fila_agencias+4); 
        fgets(fechaIn, sizeof(fechaIn), stdin);
        fechaIn[strcspn(fechaIn, "\n")] = 0;
        
        if (!fechaValida(fechaIn)) {
            setColor(COLOR_ERROR); gotoxy(10, fila_agencias+6); printf("Formato de fecha inválido. Use YYYY-MM-DD.");
            gotoxy(10, fila_agencias+7); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila_agencias+6); printf("                                           ");
            gotoxy(10, fila_agencias+7); printf("                                           ");
        }
    } while (!fechaValida(fechaIn));

    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila_agencias+6); printf("Fecha fin (YYYY-MM-DD): ");
        setColor(COLOR_NORMAL); gotoxy(33, fila_agencias+6); 
        fgets(fechaFin, sizeof(fechaFin), stdin);
        fechaFin[strcspn(fechaFin, "\n")] = 0;
        
        if (!fechaValida(fechaFin)) {
            setColor(COLOR_ERROR); gotoxy(10, fila_agencias+8); printf("Formato de fecha inválido. Use YYYY-MM-DD.");
            gotoxy(10, fila_agencias+9); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila_agencias+8); printf("                                           ");
            gotoxy(10, fila_agencias+9); printf("                                           ");
        } else if (strcmp(fechaFin, fechaIn) <= 0) {
            setColor(COLOR_ERROR); gotoxy(10, fila_agencias+8); printf("La fecha fin debe ser posterior a la fecha inicio.");
            gotoxy(10, fila_agencias+9); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila_agencias+8); printf("                                           ");
            gotoxy(10, fila_agencias+9); printf("                                           ");
        }
    } while (!fechaValida(fechaFin) || strcmp(fechaFin, fechaIn) <= 0);

    // Seleccionar coche
    int indice = mostrarCochesDisponiblesMenu(matriculas, 100);
    if (indice == -1) return;

    char *coche = matriculas[indice];
    char query[512];

    snprintf(query,
        sizeof(query),
        "INSERT INTO Reserva (FechaIn, FechaFin, PrecioTotal, Estado, ID_Clientes, ID_Agencia) "
        "VALUES ('%s', '%s', 500, 'pendiente', %d, %d)",
        fechaIn, fechaFin, id_cliente, id_agencia);

    if (mysql_query(conn, query) == 0) {
        int id_reserva = mysql_insert_id(conn);
        snprintf(query, sizeof(query), "INSERT INTO Incluye (ID_Reserva, Matricula) VALUES (%d, '%s')", id_reserva, coche);
        
        if (mysql_query(conn, query) == 0) {
            // Marcar el coche como no disponible
            snprintf(query, sizeof(query), "UPDATE Coche SET Disponible = 0 WHERE Matricula = '%s'", coche);
            mysql_query(conn, query);
            
            setColor(COLOR_EXITO); gotoxy(10, fila_agencias+10); printf("Reserva realizada con éxito. ID de reserva: %d", id_reserva);
        } else {
            setColor(COLOR_ERROR); gotoxy(10, fila_agencias+10); printf("Error en 'Incluye': %s", mysql_error(conn));
        }
    } else {
        setColor(COLOR_ERROR); gotoxy(10, fila_agencias+10); printf("Error en 'Reserva': %s", mysql_error(conn));
    }
    
    gotoxy(10, fila_agencias+12); printf("Presione cualquier tecla para continuar...");
    getch();
}

void actualizarAgencia() {
    int id_agencia;
    char nombre[50], direccion[100];
    
    mostrarEncabezado("ACTUALIZAR AGENCIA");
    
    // Mostrar lista de agencias
    setColor(COLOR_SUBTITULO); gotoxy(10, 9); printf("Agencias registradas:");
    setColor(COLOR_NORMAL);
    
    if (mysql_query(conn, "SELECT ID_Agencia, NombreA, DireccionA FROM Agencia")) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("Error al consultar agencias: %s", mysql_error(conn));
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("No hay agencias registradas.");
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    int fila = 11;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(10, fila); printf("ID: %s", row[0]);
        gotoxy(20, fila); printf("Nombre: %s", row[1]);
        gotoxy(50, fila); printf("Dirección: %s", row[2]);
        fila++;
    }
    mysql_free_result(res);
    
    // Solicitar ID de la agencia a actualizar
    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila+2); printf("ID de la agencia a actualizar: ");
        setColor(COLOR_NORMAL); gotoxy(40, fila+2); 
        scanf("%d", &id_agencia);
        limpiarBuffer();
        
        if (!verificarIDExiste("Agencia", "ID_Agencia", id_agencia)) {
            setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No existe una agencia con ese ID.");
            gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila+4); printf("                                           ");
            gotoxy(10, fila+5); printf("                                           ");
        }
    } while (!verificarIDExiste("Agencia", "ID_Agencia", id_agencia));
    
    // Obtener datos actuales de la agencia
    char query_actual[256];
    snprintf(query_actual, sizeof(query_actual), "SELECT NombreA, DireccionA FROM Agencia WHERE ID_Agencia = %d", id_agencia);
    
    if (mysql_query(conn, query_actual)) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("Error al obtener datos de la agencia: %s", mysql_error(conn));
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No se encontraron datos de la agencia.");
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        if (res) mysql_free_result(res);
        return;
    }
    
    row = mysql_fetch_row(res);
    strcpy(nombre, row[0]);
    strcpy(direccion, row[1]);
    mysql_free_result(res);
    
    // Mostrar datos actuales
    gotoxy(10, fila+4); printf("Datos actuales:");
    gotoxy(10, fila+5); printf("Nombre: %s", nombre);
    gotoxy(10, fila+6); printf("Dirección: %s", direccion);
    
    // Solicitar nuevos datos
    setColor(COLOR_CAMPO); gotoxy(10, fila+8); printf("Nuevo nombre (dejar vacío para no cambiar): ");
    setColor(COLOR_NORMAL); gotoxy(53, fila+8); 
    fgets(nombre, sizeof(nombre), stdin);
    nombre[strcspn(nombre, "\n")] = 0;
    
    setColor(COLOR_CAMPO); gotoxy(10, fila+9); printf("Nueva dirección (dejar vacío para no cambiar): ");
    setColor(COLOR_NORMAL); gotoxy(56, fila+9); 
    fgets(direccion, sizeof(direccion), stdin);
    direccion[strcspn(direccion, "\n")] = 0;
    
    // Construir consulta de actualización
    char query_update[256];
    char set_clause[200] = "";
    
    if (strlen(nombre) > 0) {
        strcat(set_clause, "NombreA = '");
        strcat(set_clause, nombre);
        strcat(set_clause, "'");
    }
    
    if (strlen(direccion) > 0) {
        if (strlen(set_clause) > 0) strcat(set_clause, ", ");
        strcat(set_clause, "DireccionA = '");
        strcat(set_clause, direccion);
        strcat(set_clause, "'");
    }
    
    if (strlen(set_clause) == 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila+11); printf("No se proporcionaron datos para actualizar.");
        gotoxy(10, fila+12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    snprintf(query_update, sizeof(query_update), "UPDATE Agencia SET %s WHERE ID_Agencia = %d", set_clause, id_agencia);
    
    if (mysql_query(conn, query_update)) {
        setColor(COLOR_ERROR); gotoxy(10, fila+11); printf("Error al actualizar agencia: %s", mysql_error(conn));
    } else {
        setColor(COLOR_EXITO); gotoxy(10, fila+11); printf("Agencia actualizada exitosamente.");
    }
    
    gotoxy(10, fila+13); printf("Presione cualquier tecla para continuar...");
    getch();
}

void agregarAgencia() {
    char nombre[50], direccion[100];
    
    mostrarEncabezado("AGREGAR NUEVA AGENCIA");
    
    // Validación del nombre
    do {
        setColor(COLOR_CAMPO); gotoxy(10, 10); printf("Nombre de la agencia: ");
        setColor(COLOR_NORMAL); gotoxy(32, 10); 
        fgets(nombre, sizeof(nombre), stdin);
        nombre[strcspn(nombre, "\n")] = 0;
        
        if (strlen(nombre) == 0) {
            setColor(COLOR_ERROR); gotoxy(10, 12); printf("El nombre no puede estar vacío.");
            gotoxy(10, 13); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, 12); printf("                                           ");
            gotoxy(10, 13); printf("                                           ");
        }
    } while (strlen(nombre) == 0);

    // Validación de la dirección
    do {
        setColor(COLOR_CAMPO); gotoxy(10, 12); printf("Dirección: ");
        setColor(COLOR_NORMAL); gotoxy(22, 12); 
        fgets(direccion, sizeof(direccion), stdin);
        direccion[strcspn(direccion, "\n")] = 0;
        
        if (strlen(direccion) == 0) {
            setColor(COLOR_ERROR); gotoxy(10, 14); printf("La dirección no puede estar vacía.");
            gotoxy(10, 15); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, 14); printf("                                           ");
            gotoxy(10, 15); printf("                                           ");
        }
    } while (strlen(direccion) == 0);

    char query[256];
    snprintf(query, sizeof(query), "INSERT INTO Agencia (NombreA, DireccionA) VALUES ('%s', '%s')", nombre, direccion);

    if (mysql_query(conn, query)) {
        setColor(COLOR_ERROR); gotoxy(10, 16); printf("Error al agregar agencia: %s", mysql_error(conn));
    } else {
        setColor(COLOR_EXITO); gotoxy(10, 16); printf("Agencia agregada exitosamente con ID: %ld", mysql_insert_id(conn));
    }
    
    setColor(COLOR_NORMAL); gotoxy(10, 18); printf("Presione cualquier tecla para continuar...");
    getch();
}

void agregarGaraje() {
    char nombre[50], direccion[100];
    
    mostrarEncabezado("AGREGAR NUEVO GARAJE");
    
    // Validación del nombre
    do {
        setColor(COLOR_CAMPO); gotoxy(10, 10); printf("Nombre del garaje: ");
        setColor(COLOR_NORMAL); gotoxy(30, 10); 
        fgets(nombre, sizeof(nombre), stdin);
        nombre[strcspn(nombre, "\n")] = 0;
        
        if (strlen(nombre) == 0) {
            setColor(COLOR_ERROR); gotoxy(10, 12); printf("El nombre no puede estar vacío.");
            gotoxy(10, 13); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, 12); printf("                                           ");
            gotoxy(10, 13); printf("                                           ");
        }
    } while (strlen(nombre) == 0);

    // Validación de la dirección
    do {
        setColor(COLOR_CAMPO); gotoxy(10, 12); printf("Dirección del garaje: ");
        setColor(COLOR_NORMAL); gotoxy(32, 12); 
        fgets(direccion, sizeof(direccion), stdin);
        direccion[strcspn(direccion, "\n")] = 0;
        
        if (strlen(direccion) == 0) {
            setColor(COLOR_ERROR); gotoxy(10, 14); printf("La dirección no puede estar vacía.");
            gotoxy(10, 15); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, 14); printf("                                           ");
            gotoxy(10, 15); printf("                                           ");
        }
    } while (strlen(direccion) == 0);

    char query[256];
    snprintf(query, sizeof(query), "INSERT INTO Garaje (NombreG, DireccionG) VALUES ('%s', '%s')", nombre, direccion);

    if (mysql_query(conn, query)) {
        setColor(COLOR_ERROR); gotoxy(10, 16); printf("Error al agregar garaje: %s", mysql_error(conn));
    } else {
        setColor(COLOR_EXITO); gotoxy(10, 16); printf("Garaje agregado exitosamente con ID: %ld", mysql_insert_id(conn));
    }
    
    setColor(COLOR_NORMAL); gotoxy(10, 18); printf("Presione cualquier tecla para continuar...");
    getch();
}

void actualizarGaraje() {
    int id_garaje;
    char nombre[50], direccion[100];
    
    mostrarEncabezado("ACTUALIZAR GARAJE");
    
    // Mostrar lista de garajes
    setColor(COLOR_SUBTITULO); gotoxy(10, 9); printf("Garajes registrados:");
    setColor(COLOR_NORMAL);
    
    if (mysql_query(conn, "SELECT ID_Garaje, NombreG, DireccionG FROM Garaje")) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("Error al consultar garajes: %s", mysql_error(conn));
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("No hay garajes registrados.");
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    int fila = 11;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(10, fila); printf("ID: %s", row[0]);
        gotoxy(20, fila); printf("Nombre: %s", row[1]);
        gotoxy(50, fila); printf("Dirección: %s", row[2]);
        fila++;
    }
    mysql_free_result(res);
    
    // Solicitar ID del garaje a actualizar
    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila+2); printf("ID del garaje a actualizar: ");
        setColor(COLOR_NORMAL); gotoxy(38, fila+2); 
        scanf("%d", &id_garaje);
        limpiarBuffer();
        
        if (!verificarIDExiste("Garaje", "ID_Garaje", id_garaje)) {
            setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No existe un garaje con ese ID.");
            gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila+4); printf("                                           ");
            gotoxy(10, fila+5); printf("                                           ");
        }
    } while (!verificarIDExiste("Garaje", "ID_Garaje", id_garaje));
    
    // Obtener datos actuales del garaje
    char query_actual[256];
    snprintf(query_actual, sizeof(query_actual), "SELECT NombreG, DireccionG FROM Garaje WHERE ID_Garaje = %d", id_garaje);
    
    if (mysql_query(conn, query_actual)) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("Error al obtener datos del garaje: %s", mysql_error(conn));
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No se encontraron datos del garaje.");
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        if (res) mysql_free_result(res);
        return;
    }
    
    row = mysql_fetch_row(res);
    strcpy(nombre, row[0]);
    strcpy(direccion, row[1]);
    mysql_free_result(res);
    
    // Mostrar datos actuales
    gotoxy(10, fila+4); printf("Datos actuales:");
    gotoxy(10, fila+5); printf("Nombre: %s", nombre);
    gotoxy(10, fila+6); printf("Dirección: %s", direccion);
    
    // Solicitar nuevos datos
    setColor(COLOR_CAMPO); gotoxy(10, fila+8); printf("Nuevo nombre (dejar vacío para no cambiar): ");
    setColor(COLOR_NORMAL); gotoxy(53, fila+8); 
    fgets(nombre, sizeof(nombre), stdin);
    nombre[strcspn(nombre, "\n")] = 0;
    
    setColor(COLOR_CAMPO); gotoxy(10, fila+9); printf("Nueva dirección (dejar vacío para no cambiar): ");
    setColor(COLOR_NORMAL); gotoxy(56, fila+9); 
    fgets(direccion, sizeof(direccion), stdin);
    direccion[strcspn(direccion, "\n")] = 0;
    
    // Construir consulta de actualización
    char query_update[256];
    char set_clause[200] = "";
    
    if (strlen(nombre) > 0) {
        strcat(set_clause, "NombreG = '");
        strcat(set_clause, nombre);
        strcat(set_clause, "'");
    }
    
    if (strlen(direccion) > 0) {
        if (strlen(set_clause) > 0) strcat(set_clause, ", ");
        strcat(set_clause, "DireccionG = '");
        strcat(set_clause, direccion);
        strcat(set_clause, "'");
    }
    
    if (strlen(set_clause) == 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila+11); printf("No se proporcionaron datos para actualizar.");
        gotoxy(10, fila+12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    snprintf(query_update, sizeof(query_update), "UPDATE Garaje SET %s WHERE ID_Garaje = %d", set_clause, id_garaje);
    
    if (mysql_query(conn, query_update)) {
        setColor(COLOR_ERROR); gotoxy(10, fila+11); printf("Error al actualizar garaje: %s", mysql_error(conn));
    } else {
        setColor(COLOR_EXITO); gotoxy(10, fila+11); printf("Garaje actualizado exitosamente.");
    }
    
    gotoxy(10, fila+13); printf("Presione cualquier tecla para continuar...");
    getch();
}

void agregarCoche() {
    char matricula[11], modelo[50], marca[50], color[30];
    int precio, id_garaje;
    
    mostrarEncabezado("AGREGAR NUEVO COCHE");
    
    // Validación de matrícula
    do {
        setColor(COLOR_CAMPO); gotoxy(10, 10); printf("Matrícula (0000-AAA): ");
        setColor(COLOR_NORMAL); gotoxy(32, 10); 
        fgets(matricula, sizeof(matricula), stdin);
        matricula[strcspn(matricula, "\n")] = 0;
        
        if (!esMatriculaValida(matricula)) {
            setColor(COLOR_ERROR); gotoxy(10, 12); printf("Formato de matrícula inválido. Use 0000-AAA (4 números, guión, 3 letras).");
            gotoxy(10, 13); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, 12); printf("                                                                   ");
            gotoxy(10, 13); printf("                                           ");
        } else {
            // Verificar si la matrícula ya existe
            char query_check[100];
            snprintf(query_check, sizeof(query_check), "SELECT 1 FROM Coche WHERE Matricula = '%s'", matricula);
            if (mysql_query(conn, query_check) == 0) {
                res = mysql_store_result(conn);
                if (res && mysql_num_rows(res) > 0) {
                    setColor(COLOR_ERROR); gotoxy(10, 12); printf("Ya existe un coche con esa matrícula.");
                    gotoxy(10, 13); printf("Presione cualquier tecla para continuar...");
                    getch();
                    gotoxy(10, 12); printf("                                           ");
                    gotoxy(10, 13); printf("                                           ");
                    matricula[0] = '\0'; // Forzar repetición
                }
                if (res) mysql_free_result(res);
            }
        }
    } while (!esMatriculaValida(matricula));

    // Modelo
    setColor(COLOR_CAMPO); gotoxy(10, 12); printf("Modelo: ");
    setColor(COLOR_NORMAL); gotoxy(19, 12); 
    fgets(modelo, sizeof(modelo), stdin);
    modelo[strcspn(modelo, "\n")] = 0;
    
    // Marca
    setColor(COLOR_CAMPO); gotoxy(10, 14); printf("Marca: ");
    setColor(COLOR_NORMAL); gotoxy(18, 14); 
    fgets(marca, sizeof(marca), stdin);
    marca[strcspn(marca, "\n")] = 0;
    
    // Color
    setColor(COLOR_CAMPO); gotoxy(10, 16); printf("Color: ");
    setColor(COLOR_NORMAL); gotoxy(18, 16); 
    fgets(color, sizeof(color), stdin);
    color[strcspn(color, "\n")] = 0;
    
    // Precio
    do {
        setColor(COLOR_CAMPO); gotoxy(10, 18); printf("Precio por día: ");
        setColor(COLOR_NORMAL); gotoxy(26, 18); 
        scanf("%d", &precio);
        limpiarBuffer();
        
        if (precio <= 0) {
            setColor(COLOR_ERROR); gotoxy(10, 20); printf("El precio debe ser mayor que cero.");
            gotoxy(10, 21); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, 20); printf("                                           ");
            gotoxy(10, 21); printf("                                           ");
        }
    } while (precio <= 0);
    
    // Mostrar garajes disponibles
    setColor(COLOR_SUBTITULO); gotoxy(10, 20); printf("Garajes disponibles:");
    setColor(COLOR_NORMAL);
    
    if (mysql_query(conn, "SELECT ID_Garaje, NombreG FROM Garaje")) {
        setColor(COLOR_ERROR); gotoxy(10, 22); printf("Error al consultar garajes: %s", mysql_error(conn));
        gotoxy(10, 23); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res) {
        setColor(COLOR_ERROR); gotoxy(10, 22); printf("No hay garajes registrados.");
        gotoxy(10, 23); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    int fila_garajes = 22;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(10, fila_garajes); printf("ID: %s", row[0]);
        gotoxy(20, fila_garajes); printf("Nombre: %s", row[1]);
        fila_garajes++;
    }
    mysql_free_result(res);
    
    // Solicitar ID del garaje
    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila_garajes+2); printf("ID del garaje: ");
        setColor(COLOR_NORMAL); gotoxy(25, fila_garajes+2); 
        scanf("%d", &id_garaje);
        limpiarBuffer();
        
        if (!verificarIDExiste("Garaje", "ID_Garaje", id_garaje)) {
            setColor(COLOR_ERROR); gotoxy(10, fila_garajes+4); printf("No existe un garaje con ese ID.");
            gotoxy(10, fila_garajes+5); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila_garajes+4); printf("                                           ");
            gotoxy(10, fila_garajes+5); printf("                                           ");
        }
    } while (!verificarIDExiste("Garaje", "ID_Garaje", id_garaje));
    
    // Insertar coche
    char query[512];
    snprintf(query, sizeof(query),
        "INSERT INTO Coche (Matricula, Modelo, Marca, Color, Precio, ID_Garaje, Disponible, Gasolina) "
        "VALUES ('%s', '%s', '%s', '%s', %d, %d, 1, 100)",
        matricula, modelo, marca, color, precio, id_garaje);
    
    if (mysql_query(conn, query)) {
        setColor(COLOR_ERROR); gotoxy(10, fila_garajes+4); printf("Error al agregar coche: %s", mysql_error(conn));
    } else {
        setColor(COLOR_EXITO); gotoxy(10, fila_garajes+4); printf("Coche agregado exitosamente.");
    }
    
    gotoxy(10, fila_garajes+6); printf("Presione cualquier tecla para continuar...");
    getch();
}

void actualizarCoche() {
    char matricula[11], nuevoModelo[50], nuevaMarca[50], nuevoColor[30];
    int nuevoPrecio, nuevoGaraje;
    
    mostrarEncabezado("ACTUALIZAR COCHE");
    
    // Mostrar lista de coches
    setColor(COLOR_SUBTITULO); gotoxy(10, 9); printf("Coches registrados:");
    setColor(COLOR_NORMAL);
    
    if (mysql_query(conn, "SELECT Matricula, Modelo, Marca FROM Coche")) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("Error al consultar coches: %s", mysql_error(conn));
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("No hay coches registrados.");
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    int fila = 11;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(10, fila); printf("Matrícula: %s", row[0]);
        gotoxy(25, fila); printf("Modelo: %s", row[1]);
        gotoxy(50, fila); printf("Marca: %s", row[2]);
        fila++;
    }
    mysql_free_result(res);
    
    // Solicitar matrícula del coche a actualizar
    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila+2); printf("Matrícula del coche a actualizar: ");
        setColor(COLOR_NORMAL); gotoxy(42, fila+2); 
        fgets(matricula, sizeof(matricula), stdin);
        matricula[strcspn(matricula, "\n")] = 0;
        
        if (!esMatriculaValida(matricula)) {
            setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("Formato de matrícula inválido. Use 0000-AAA.");
            gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila+4); printf("                                           ");
            gotoxy(10, fila+5); printf("                                           ");
        } else {
            // Verificar si la matrícula existe
            char query_check[100];
            snprintf(query_check, sizeof(query_check), "SELECT 1 FROM Coche WHERE Matricula = '%s'", matricula);
            if (mysql_query(conn, query_check) != 0 || 
                (res = mysql_store_result(conn)) == NULL || 
                mysql_num_rows(res) == 0) {
                setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No existe un coche con esa matrícula.");
                gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
                getch();
                gotoxy(10, fila+4); printf("                                           ");
                gotoxy(10, fila+5); printf("                                           ");
                matricula[0] = '\0'; // Forzar repetición
            }
            if (res) mysql_free_result(res);
        }
    } while (!esMatriculaValida(matricula) || !verificarIDExiste("Coche", "Matricula", matricula));
    
    // Obtener datos actuales del coche
    char query_actual[256];
    snprintf(query_actual, sizeof(query_actual), 
             "SELECT Modelo, Marca, Color, Precio, ID_Garaje FROM Coche WHERE Matricula = '%s'", matricula);
    
    if (mysql_query(conn, query_actual)) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("Error al obtener datos del coche: %s", mysql_error(conn));
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No se encontraron datos del coche.");
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        if (res) mysql_free_result(res);
        return;
    }
    
    row = mysql_fetch_row(res);
    strcpy(nuevoModelo, row[0]);
    strcpy(nuevaMarca, row[1]);
    strcpy(nuevoColor, row[2]);
    nuevoPrecio = atoi(row[3]);
    nuevoGaraje = atoi(row[4]);
    mysql_free_result(res);
    
    // Mostrar datos actuales
    gotoxy(10, fila+4); printf("Datos actuales:");
    gotoxy(10, fila+5); printf("Modelo: %s", nuevoModelo);
    gotoxy(10, fila+6); printf("Marca: %s", nuevaMarca);
    gotoxy(10, fila+7); printf("Color: %s", nuevoColor);
    gotoxy(10, fila+8); printf("Precio: %d", nuevoPrecio);
    gotoxy(10, fila+9); printf("ID Garaje: %d", nuevoGaraje);
    
    // Solicitar nuevos datos
    setColor(COLOR_CAMPO); gotoxy(10, fila+11); printf("Nuevo modelo (dejar vacío para no cambiar): ");
    setColor(COLOR_NORMAL); gotoxy(53, fila+11); 
    fgets(nuevoModelo, sizeof(nuevoModelo), stdin);
    nuevoModelo[strcspn(nuevoModelo, "\n")] = 0;
    
    setColor(COLOR_CAMPO); gotoxy(10, fila+12); printf("Nueva marca (dejar vacío para no cambiar): ");
    setColor(COLOR_NORMAL); gotoxy(52, fila+12); 
    fgets(nuevaMarca, sizeof(nuevaMarca), stdin);
    nuevaMarca[strcspn(nuevaMarca, "\n")] = 0;
    
    setColor(COLOR_CAMPO); gotoxy(10, fila+13); printf("Nuevo color (dejar vacío para no cambiar): ");
    setColor(COLOR_NORMAL); gotoxy(52, fila+13); 
    fgets(nuevoColor, sizeof(nuevoColor), stdin);
    nuevoColor[strcspn(nuevoColor, "\n")] = 0;
    
    char inputPrecio[20];
    setColor(COLOR_CAMPO); gotoxy(10, fila+14); printf("Nuevo precio (dejar vacío para no cambiar): ");
    setColor(COLOR_NORMAL); gotoxy(53, fila+14); 
    fgets(inputPrecio, sizeof(inputPrecio), stdin);
    inputPrecio[strcspn(inputPrecio, "\n")] = 0;
    
    // Mostrar garajes disponibles
    setColor(COLOR_SUBTITULO); gotoxy(10, fila+16); printf("Garajes disponibles:");
    setColor(COLOR_NORMAL);
    
    if (mysql_query(conn, "SELECT ID_Garaje, NombreG FROM Garaje")) {
        setColor(COLOR_ERROR); gotoxy(10, fila+18); printf("Error al consultar garajes: %s", mysql_error(conn));
        gotoxy(10, fila+19); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res) {
        setColor(COLOR_ERROR); gotoxy(10, fila+18); printf("No hay garajes registrados.");
        gotoxy(10, fila+19); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    int fila_garajes = fila+18;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(10, fila_garajes); printf("ID: %s", row[0]);
        gotoxy(20, fila_garajes); printf("Nombre: %s", row[1]);
        fila_garajes++;
    }
    mysql_free_result(res);
    
    char inputGaraje[20];
    setColor(COLOR_CAMPO); gotoxy(10, fila_garajes+2); printf("Nuevo ID Garaje (dejar vacío para no cambiar): ");
    setColor(COLOR_NORMAL); gotoxy(55, fila_garajes+2); 
    fgets(inputGaraje, sizeof(inputGaraje), stdin);
    inputGaraje[strcspn(inputGaraje, "\n")] = 0;
    
    // Construir consulta de actualización
    char query_update[512];
    char set_clause[400] = "";
    
    if (strlen(nuevoModelo) > 0) {
        strcat(set_clause, "Modelo = '");
        strcat(set_clause, nuevoModelo);
        strcat(set_clause, "'");
    }
    
    if (strlen(nuevaMarca) > 0) {
        if (strlen(set_clause) > 0) strcat(set_clause, ", ");
        strcat(set_clause, "Marca = '");
        strcat(set_clause, nuevaMarca);
        strcat(set_clause, "'");
    }
    
    if (strlen(nuevoColor) > 0) {
        if (strlen(set_clause) > 0) strcat(set_clause, ", ");
        strcat(set_clause, "Color = '");
        strcat(set_clause, nuevoColor);
        strcat(set_clause, "'");
    }
    
    if (strlen(inputPrecio) > 0) {
        if (strlen(set_clause) > 0) strcat(set_clause, ", ");
        strcat(set_clause, "Precio = ");
        strcat(set_clause, inputPrecio);
    }
    
    if (strlen(inputGaraje) > 0) {
        if (strlen(set_clause) > 0) strcat(set_clause, ", ");
        strcat(set_clause, "ID_Garaje = ");
        strcat(set_clause, inputGaraje);
    }
    
    if (strlen(set_clause) == 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila_garajes+4); printf("No se proporcionaron datos para actualizar.");
        gotoxy(10, fila_garajes+5); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    snprintf(query_update, sizeof(query_update), "UPDATE Coche SET %s WHERE Matricula = '%s'", set_clause, matricula);
    
    if (mysql_query(conn, query_update)) {
        setColor(COLOR_ERROR); gotoxy(10, fila_garajes+4); printf("Error al actualizar coche: %s", mysql_error(conn));
    } else {
        setColor(COLOR_EXITO); gotoxy(10, fila_garajes+4); printf("Coche actualizado exitosamente.");
    }
    
    gotoxy(10, fila_garajes+6); printf("Presione cualquier tecla para continuar...");
    getch();
}

void entregarCoche() {
    int id_reserva, gasolinaRestante;
    
    mostrarEncabezado("ENTREGAR COCHE");
    
    // Mostrar reservas pendientes
    setColor(COLOR_SUBTITULO); gotoxy(10, 9); printf("Reservas pendientes:");
    setColor(COLOR_NORMAL);
    
    if (mysql_query(conn, "SELECT R.ID_Reserva, C.Nombre, C.Apellido, Co.Modelo, Co.Matricula "
                         "FROM Reserva R "
                         "JOIN Clientes C ON R.ID_Clientes = C.ID_Clientes "
                         "JOIN Incluye I ON R.ID_Reserva = I.ID_Reserva "
                         "JOIN Coche Co ON I.Matricula = Co.Matricula "
                         "WHERE R.Estado = 'pendiente'")) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("Error al consultar reservas: %s", mysql_error(conn));
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("No hay reservas pendientes.");
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        if (res) mysql_free_result(res);
        return;
    }
    
    int fila = 11;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(10, fila); printf("ID Reserva: %s", row[0]);
        gotoxy(25, fila); printf("Cliente: %s %s", row[1], row[2]);
        gotoxy(50, fila); printf("Coche: %s (%s)", row[3], row[4]);
        fila++;
    }
    mysql_free_result(res);
    
    // Solicitar ID de la reserva
    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila+2); printf("ID de la reserva a entregar: ");
        setColor(COLOR_NORMAL); gotoxy(38, fila+2); 
        scanf("%d", &id_reserva);
        limpiarBuffer();
        
        if (!verificarIDExiste("Reserva", "ID_Reserva", id_reserva)) {
            setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No existe una reserva con ese ID.");
            gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila+4); printf("                                           ");
            gotoxy(10, fila+5); printf("                                           ");
        }
    } while (!verificarIDExiste("Reserva", "ID_Reserva", id_reserva));
    
    // Verificar que la reserva esté pendiente
    char query_check[256];
    snprintf(query_check, sizeof(query_check), 
             "SELECT 1 FROM Reserva WHERE ID_Reserva = %d AND Estado = 'pendiente'", id_reserva);
    
    if (mysql_query(conn, query_check) != 0 || 
        (res = mysql_store_result(conn)) == NULL || 
        mysql_num_rows(res) == 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("La reserva no está pendiente o no existe.");
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        if (res) mysql_free_result(res);
        return;
    }
    mysql_free_result(res);
    
    // Obtener matrícula del coche asociado
    char query_matricula[256];
    snprintf(query_matricula, sizeof(query_matricula),
             "SELECT I.Matricula FROM Incluye I WHERE I.ID_Reserva = %d", id_reserva);
    
    if (mysql_query(conn, query_matricula) != 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("Error al obtener matrícula: %s", mysql_error(conn));
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No se encontró el coche asociado a la reserva.");
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        if (res) mysql_free_result(res);
        return;
    }
    
    row = mysql_fetch_row(res);
    char matricula[11];
    strncpy(matricula, row[0], 10);
    matricula[10] = '\0';
    mysql_free_result(res);
    
    // Preguntar nivel de gasolina
    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila+4); printf("Nivel de gasolina al devolver (0-100): ");
        setColor(COLOR_NORMAL); gotoxy(45, fila+4); 
        scanf("%d", &gasolinaRestante);
        limpiarBuffer();
        
        if (gasolinaRestante < 0 || gasolinaRestante > 100) {
            setColor(COLOR_ERROR); gotoxy(10, fila+6); printf("El nivel de gasolina debe estar entre 0 y 100.");
            gotoxy(10, fila+7); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila+6); printf("                                           ");
            gotoxy(10, fila+7); printf("                                           ");
        }
    } while (gasolinaRestante < 0 || gasolinaRestante > 100);
    
    // Actualizar estado de la reserva y gasolina del coche
    char query_update[512];
    snprintf(query_update, sizeof(query_update),
             "START TRANSACTION; "
             "UPDATE Reserva SET Estado = 'entregado' WHERE ID_Reserva = %d; "
             "UPDATE Coche SET Disponible = 1, Gasolina = %d WHERE Matricula = '%s'; "
             "COMMIT;",
             id_reserva, gasolinaRestante, matricula);
    
    if (mysql_query(conn, query_update)) {
        setColor(COLOR_ERROR); gotoxy(10, fila+6); printf("Error al actualizar datos: %s", mysql_error(conn));
    } else {
        setColor(COLOR_EXITO); gotoxy(10, fila+6); printf("Coche entregado exitosamente.");
    }
    
    gotoxy(10, fila+8); printf("Presione cualquier tecla para continuar...");
    getch();
}

void consultarReservasPorCliente() {
    int id_cliente;
    
    mostrarEncabezado("CONSULTAR RESERVAS POR CLIENTE");
    
    // Mostrar lista de clientes
    setColor(COLOR_SUBTITULO); gotoxy(10, 9); printf("Clientes registrados:");
    setColor(COLOR_NORMAL);
    
    if (mysql_query(conn, "SELECT ID_Clientes, Nombre, Apellido FROM Clientes ORDER BY Apellido")) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("Error al consultar clientes: %s", mysql_error(conn));
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("No hay clientes registrados.");
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    int fila = 11;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(10, fila); printf("ID: %s", row[0]);
        gotoxy(20, fila); printf("Nombre: %s %s", row[1], row[2]);
        fila++;
    }
    mysql_free_result(res);
    
    // Solicitar ID del cliente
    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila+2); printf("ID del cliente: ");
        setColor(COLOR_NORMAL); gotoxy(26, fila+2); 
        scanf("%d", &id_cliente);
        limpiarBuffer();
        
        if (!verificarIDExiste("Clientes", "ID_Clientes", id_cliente)) {
            setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No existe un cliente con ese ID.");
            gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila+4); printf("                                           ");
            gotoxy(10, fila+5); printf("                                           ");
        }
    } while (!verificarIDExiste("Clientes", "ID_Clientes", id_cliente));
    
    // Consultar reservas del cliente
    char query[512];
    snprintf(query,
        sizeof(query),
        "SELECT R.ID_Reserva, R.FechaIn, R.FechaFin, R.Estado, Co.Matricula, Co.Modelo, A.NombreA "
        "FROM Reserva R "
        "JOIN Incluye I ON R.ID_Reserva = I.ID_Reserva "
        "JOIN Coche Co ON I.Matricula = Co.Matricula "
        "JOIN Agencia A ON R.ID_Agencia = A.ID_Agencia "
        "WHERE R.ID_Clientes = %d", id_cliente);

    if (mysql_query(conn, query) != 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("Error al consultar reservas: %s", mysql_error(conn));
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) {
        setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("El cliente no tiene reservas.");
        gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
        getch();
        if (res) mysql_free_result(res);
        return;
    }
    
    // Mostrar encabezados
    setColor(COLOR_SUBTITULO);
    gotoxy(10, fila+4); printf("ID Reserva");
    gotoxy(22, fila+4); printf("Fechas");
    gotoxy(40, fila+4); printf("Estado");
    gotoxy(55, fila+4); printf("Coche");
    gotoxy(75, fila+4); printf("Agencia");
    setColor(COLOR_NORMAL);
    
    fila += 6;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(10, fila); printf("%s", row[0]);
        gotoxy(22, fila); printf("%s a %s", row[1], row[2]);
        gotoxy(40, fila); printf("%s", row[3]);
        gotoxy(55, fila); printf("%s (%s)", row[5], row[4]);
        gotoxy(75, fila); printf("%s", row[6]);
        fila++;
    }
    mysql_free_result(res);
    
    gotoxy(10, fila+2); printf("Presione cualquier tecla para continuar...");
    getch();
}

void eliminarCliente() {
    int id_cliente;
    
    mostrarEncabezado("ELIMINAR CLIENTE");
    
    // Mostrar lista de clientes
    setColor(COLOR_SUBTITULO); gotoxy(10, 9); printf("Clientes registrados:");
    setColor(COLOR_NORMAL);
    
    if (mysql_query(conn, "SELECT ID_Clientes, Nombre, Apellido FROM Clientes ORDER BY Apellido")) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("Error al consultar clientes: %s", mysql_error(conn));
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    res = mysql_store_result(conn);
    if (!res) {
        setColor(COLOR_ERROR); gotoxy(10, 11); printf("No hay clientes registrados.");
        gotoxy(10, 12); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    int fila = 11;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(10, fila); printf("ID: %s", row[0]);
        gotoxy(20, fila); printf("Nombre: %s %s", row[1], row[2]);
        fila++;
    }
    mysql_free_result(res);
    
    // Solicitar ID del cliente a eliminar
    do {
        setColor(COLOR_CAMPO); gotoxy(10, fila+2); printf("ID del cliente a eliminar: ");
        setColor(COLOR_NORMAL); gotoxy(35, fila+2); 
        scanf("%d", &id_cliente);
        limpiarBuffer();
        
        if (!verificarIDExiste("Clientes", "ID_Clientes", id_cliente)) {
            setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("No existe un cliente con ese ID.");
            gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, fila+4); printf("                                           ");
            gotoxy(10, fila+5); printf("                                           ");
        }
    } while (!verificarIDExiste("Clientes", "ID_Clientes", id_cliente));
    
    // Verificar si el cliente tiene reservas pendientes
    char query_check[256];
    snprintf(query_check, sizeof(query_check), 
             "SELECT 1 FROM Reserva WHERE ID_Clientes = %d AND Estado = 'pendiente'", id_cliente);
    
    if (mysql_query(conn, query_check) == 0) {
        res = mysql_store_result(conn);
        if (res && mysql_num_rows(res) > 0) {
            setColor(COLOR_ERROR); gotoxy(10, fila+4); printf("El cliente tiene reservas pendientes. No se puede eliminar.");
            gotoxy(10, fila+5); printf("Presione cualquier tecla para continuar...");
            getch();
            mysql_free_result(res);
            return;
        }
        if (res) mysql_free_result(res);
    }
    
    // Confirmar eliminación
    setColor(COLOR_CAMPO); gotoxy(10, fila+4); printf("¿Está seguro que desea eliminar este cliente? (s/n): ");
    setColor(COLOR_NORMAL); gotoxy(60, fila+4); 
    char confirmacion = getchar();
    limpiarBuffer();
    
    if (tolower(confirmacion) != 's') {
        setColor(COLOR_ERROR); gotoxy(10, fila+6); printf("Eliminación cancelada.");
        gotoxy(10, fila+7); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }
    
    // Eliminar cliente
    char query[100];
    snprintf(query, sizeof(query), "DELETE FROM Clientes WHERE ID_Clientes = %d", id_cliente);
    
    if (mysql_query(conn, query)) {
        setColor(COLOR_ERROR); gotoxy(10, fila+6); printf("Error al eliminar cliente: %s", mysql_error(conn));
    } else {
        setColor(COLOR_EXITO); gotoxy(10, fila+6); printf("Cliente eliminado exitosamente.");
    }
    
    gotoxy(10, fila+8); printf("Presione cualquier tecla para continuar...");
    getch();
}

void mostrarTabla() {
    int opc;
    
    mostrarEncabezado("CONSULTAR TABLAS");
    
    setColor(COLOR_SUBTITULO); gotoxy(10, 9); printf("Tablas disponibles:");
    setColor(COLOR_NORMAL);
    gotoxy(10, 11); printf("1. Clientes");
    gotoxy(10, 12); printf("2. Coches");
    gotoxy(10, 13); printf("3. Agencias");
    gotoxy(10, 14); printf("4. Garajes");
    gotoxy(10, 15); printf("5. Reservas");
    gotoxy(10, 16); printf("6. Incluye");
    
    do {
        setColor(COLOR_CAMPO); gotoxy(10, 18); printf("Seleccione una tabla (1-6): ");
        setColor(COLOR_NORMAL); gotoxy(38, 18); 
        scanf("%d", &opc);
        limpiarBuffer();
        
        if (opc < 1 || opc > 6) {
            setColor(COLOR_ERROR); gotoxy(10, 20); printf("Opción inválida. Intente nuevamente.");
            gotoxy(10, 21); printf("Presione cualquier tecla para continuar...");
            getch();
            gotoxy(10, 20); printf("                                           ");
            gotoxy(10, 21); printf("                                           ");
        }
    } while (opc < 1 || opc > 6);

    char query[512];
    const char *titulo;

    switch(opc) {
        case 1:
            strcpy(query, "SELECT ID_Clientes, Nombre, Apellido FROM Clientes");
            titulo = "CLIENTES";
            break;
        case 2:
            strcpy(query, "SELECT Matricula, Modelo, Marca, Color, Precio, Disponible, Gasolina, ID_Garaje FROM Coche");
            titulo = "COCHES";
            break;
        case 3:
            strcpy(query, "SELECT ID_Agencia, NombreA, DireccionA FROM Agencia");
            titulo = "AGENCIAS";
            break;
        case 4:
            strcpy(query, "SELECT ID_Garaje, NombreG, DireccionG FROM Garaje");
            titulo = "GARAJES";
            break;
        case 5:
            strcpy(query, "SELECT ID_Reserva, FechaIn, FechaFin, PrecioTotal, Estado, ID_Clientes, ID_Agencia FROM Reserva");
            titulo = "RESERVAS";
            break;
        case 6:
            strcpy(query, "SELECT ID_Reserva, Matricula FROM Incluye");
            titulo = "INCLUYE";
            break;
    }

    system("cls");
    mostrarEncabezado(titulo);
    
    if (mysql_query(conn, query) != 0) {
        setColor(COLOR_ERROR); gotoxy(10, 10); printf("Error: %s", mysql_error(conn));
        gotoxy(10, 11); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }

    res = mysql_store_result(conn);
    if (!res) {
        setColor(COLOR_ERROR); gotoxy(10, 10); printf("La tabla está vacía.");
        gotoxy(10, 11); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }

    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);

    // Mostrar encabezados
    setColor(COLOR_SUBTITULO);
    for (int i = 0; i < num_fields; i++) {
        gotoxy(10 + i * 20, 10); printf("%-15s", fields[i].name);
    }
    setColor(COLOR_NORMAL);
    
    // Mostrar datos
    int fila = 12;
    while ((row = mysql_fetch_row(res)) != NULL) {
        for (int i = 0; i < num_fields; i++) {
            gotoxy(10 + i * 20, fila); printf("%-15s", row[i] ? row[i] : "NULL");
        }
        fila++;
        
        // Pausa cada 20 registros
        if (fila % 20 == 0) {
            gotoxy(10, fila+2); printf("Presione cualquier tecla para continuar...");
            getch();
            fila += 4;
            
            // Volver a mostrar encabezados
            setColor(COLOR_SUBTITULO);
            for (int i = 0; i < num_fields; i++) {
                gotoxy(10 + i * 20, fila); printf("%-15s", fields[i].name);
            }
            setColor(COLOR_NORMAL);
            fila += 2;
        }
    }

    mysql_free_result(res);
    gotoxy(10, fila+2); printf("Fin de los registros. Presione cualquier tecla para continuar...");
    getch();
}

void mostrarReservasConJoin() {
    mostrarEncabezado("RESERVAS CON DATOS COMPLETOS");
    
    const char *query =
        "SELECT R.ID_Reserva, R.FechaIn, R.FechaFin, R.Estado, "
        "CONCAT(C.Nombre, ' ', C.Apellido) AS Cliente, "
        "A.NombreA AS Agencia, CONCAT(Co.Marca, ' ', Co.Modelo) AS Coche, Co.Matricula "
        "FROM Reserva R "
        "INNER JOIN Clientes C ON R.ID_Clientes = C.ID_Clientes "
        "INNER JOIN Agencia A ON R.ID_Agencia = A.ID_Agencia "
        "INNER JOIN Incluye I ON R.ID_Reserva = I.ID_Reserva "
        "INNER JOIN Coche Co ON I.Matricula = Co.Matricula "
        "ORDER BY R.FechaIn DESC";

    if (mysql_query(conn, query) != 0) {
        setColor(COLOR_ERROR); gotoxy(10, 10); printf("Error al ejecutar consulta: %s", mysql_error(conn));
        gotoxy(10, 11); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }

    res = mysql_store_result(conn);
    if (!res) {
        setColor(COLOR_ERROR); gotoxy(10, 10); printf("No hay resultados.");
        gotoxy(10, 11); printf("Presione cualquier tecla para continuar...");
        getch();
        return;
    }

    // Mostrar encabezados
    setColor(COLOR_SUBTITULO);
    gotoxy(5, 10); printf("ID");
    gotoxy(10, 10); printf("Fechas");
    gotoxy(30, 10); printf("Estado");
    gotoxy(45, 10); printf("Cliente");
    gotoxy(65, 10); printf("Agencia");
    gotoxy(85, 10); printf("Coche");
    setColor(COLOR_NORMAL);

    // Mostrar datos
    int fila = 12;
    while ((row = mysql_fetch_row(res)) != NULL) {
        gotoxy(5, fila); printf("%s", row[0]);
        gotoxy(10, fila); printf("%s a %s", row[1], row[2]);
        gotoxy(30, fila); printf("%s", row[3]);
        gotoxy(45, fila); printf("%s", row[4]);
        gotoxy(65, fila); printf("%s", row[5]);
        gotoxy(85, fila); printf("%s (%s)", row[6], row[7]);
        fila++;
        
        // Pausa cada 20 registros
        if (fila % 20 == 0) {
            gotoxy(5, fila+2); printf("Presione cualquier tecla para continuar...");
            getch();
            fila += 4;
            
            // Volver a mostrar encabezados
            setColor(COLOR_SUBTITULO);
            gotoxy(5, fila); printf("ID");
            gotoxy(10, fila); printf("Fechas");
            gotoxy(30, fila); printf("Estado");
            gotoxy(45, fila); printf("Cliente");
            gotoxy(65, fila); printf("Agencia");
            gotoxy(85, fila); printf("Coche");
            setColor(COLOR_NORMAL);
            fila += 2;
        }
    }

    mysql_free_result(res);
    gotoxy(5, fila+2); printf("Fin de los registros. Presione cualquier tecla para continuar...");
    getch();
}

void menu() {
    int opc;
    do {
        system("cls");
        mostrarEncabezado("MENÚ PRINCIPAL");
        
        setColor(COLOR_SUBTITULO); gotoxy(35, 9); printf("MENÚ DE OPCIONES");
        setColor(COLOR_NORMAL);
        
        gotoxy(30, 11); printf("1. Agregar cliente");
        gotoxy(30, 12); printf("2. Actualizar cliente");
        gotoxy(30, 13); printf("3. Eliminar cliente");
        gotoxy(30, 14); printf("4. Agregar coche");
        gotoxy(30, 15); printf("5. Actualizar coche");
        gotoxy(30, 16); printf("6. Agregar agencia");
        gotoxy(30, 17); printf("7. Actualizar agencia");
        gotoxy(30, 18); printf("8. Agregar garaje");
        gotoxy(30, 19); printf("9. Actualizar garaje");
        gotoxy(30, 20); printf("10. Hacer reserva");
        gotoxy(30, 21); printf("11. Entregar coche");
        gotoxy(30, 22); printf("12. Consultar reservas por cliente");
        gotoxy(30, 23); printf("13. Mostrar tabla");
        gotoxy(30, 24); printf("14. Mostrar reservas con JOIN");
        gotoxy(30, 25); printf("15. Salir");
        
        setColor(COLOR_CAMPO); gotoxy(30, 27); printf("Seleccione una opción: ");
        setColor(COLOR_NORMAL); gotoxy(52, 27); 
        scanf("%d", &opc);
        limpiarBuffer();

        switch (opc) {
            case 1: agregarCliente(); break;
            case 2: actualizarCliente(); break;
            case 3: eliminarCliente(); break;
            case 4: agregarCoche(); break;
            case 5: actualizarCoche(); break;
            case 6: agregarAgencia(); break;
            case 7: actualizarAgencia(); break;
            case 8: agregarGaraje(); break;
            case 9: actualizarGaraje(); break;
            case 10: hacerReserva(); break;
            case 11: entregarCoche(); break;
            case 12: consultarReservasPorCliente(); break;
            case 13: mostrarTabla(); break;
            case 14: mostrarReservasConJoin(); break;
            case 15: printf("Saliendo...\n"); break;
            default: 
                setColor(COLOR_ERROR); gotoxy(30, 29); printf("Opción inválida. Intente nuevamente.");
                gotoxy(30, 30); printf("Presione cualquier tecla para continuar...");
                getch();
        }
    } while (opc != 15);
}

int main() {
    // Configurar la codificación de la consola a UTF-8 para caracteres especiales
    SetConsoleOutputCP(65001);
    
    conectarBD();
    menu();
    mysql_close(conn);
    return 0;
}