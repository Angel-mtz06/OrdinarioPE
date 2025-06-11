#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;

// Función para limpiar buffer stdin
void limpiarBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void conectarBD() {
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Error al iniciar MySQL\n");
        exit(1);
    }
    if (!mysql_real_connect(conn, "localhost", "root", "", "AgenciaAlquiler", 0, NULL, 0)) {
        fprintf(stderr, "Error de conexión: %s\n", mysql_error(conn));
        exit(1);
    }
}

void agregarCliente() {
    char nombre[50], apellido[50];

    printf("Nombre: ");
    fgets(nombre, sizeof(nombre), stdin);
    nombre[strcspn(nombre, "\n")] = 0; // Eliminar salto de línea

    printf("Apellido: ");
    fgets(apellido, sizeof(apellido), stdin);
    apellido[strcspn(apellido, "\n")] = 0;

    char query[256];
    snprintf(query, sizeof(query), "INSERT INTO Clientes (Nombre, Apellido) VALUES ('%s', '%s')", nombre, apellido);

    if (mysql_query(conn, query) == 0)
        printf("Cliente agregado.\n");
    else
        printf("Error: %s\n", mysql_error(conn));
}

// Mostrar coches disponibles con índice para seleccionar
int mostrarCochesDisponiblesMenu(char matriculas[][11], int maxCoches) {
    if (mysql_query(conn, "SELECT Matricula, Modelo, Marca FROM Coche")) {
        fprintf(stderr, "Error: %s\n", mysql_error(conn));
        return -1;
    }
    res = mysql_store_result(conn);
    if (!res) return -1;

    int i = 0;
    printf("Coches disponibles:\n");
    while ((row = mysql_fetch_row(res)) != NULL && i < maxCoches) {
        printf("%d. Matrícula: %s | Modelo: %s | Marca: %s\n", i + 1, row[0], row[1], row[2]);
        strncpy(matriculas[i], row[0], 10);
        matriculas[i][10] = '\0';  // asegurar terminación
        i++;
    }
    mysql_free_result(res);

    if (i == 0) {
        printf("No hay coches disponibles.\n");
        return -1;
    }

    int opcion;
    printf("Seleccione coche (1-%d): ", i);
    scanf("%d", &opcion);
    limpiarBuffer();

    if (opcion < 1 || opcion > i) {
        printf("Opción inválida.\n");
        return -1;
    }
    return opcion - 1; // índice en array
}

void hacerReserva() {
    int id_cliente, id_agencia;
    char fechaIn[11], fechaFin[11];
    char matriculas[100][11]; // max 100 coches para simplicidad

    printf("ID Cliente: ");
    scanf("%d", &id_cliente);
    limpiarBuffer();

    printf("ID Agencia: ");
    scanf("%d", &id_agencia);
    limpiarBuffer();

    printf("Fecha inicio (YYYY-MM-DD): ");
    fgets(fechaIn, sizeof(fechaIn), stdin);
    fechaIn[strcspn(fechaIn, "\n")] = 0;

    printf("Fecha fin (YYYY-MM-DD): ");
    fgets(fechaFin, sizeof(fechaFin), stdin);
    fechaFin[strcspn(fechaFin, "\n")] = 0;

    int indice = mostrarCochesDisponiblesMenu(matriculas, 100);
    if (indice == -1) return;

    char *coche = matriculas[indice];
    char query[512];

    // Para simplificar precio fijo
    snprintf(query,
        sizeof(query),
        "INSERT INTO Reserva (FechaIn, FechaFin, PrecioTotal, Estado, ID_Clientes, ID_Agencia) "
        "VALUES ('%s', '%s', 500, 'pendiente', %d, %d)",
        fechaIn, fechaFin, id_cliente, id_agencia);

    if (mysql_query(conn, query) == 0) {
        int id_reserva = mysql_insert_id(conn);
        snprintf(query, sizeof(query), "INSERT INTO Incluye (ID_Reserva, Matricula) VALUES (%d, '%s')", id_reserva, coche);
        if (mysql_query(conn, query) == 0)
            printf("Reserva realizada con ID: %d\n", id_reserva);
        else
            printf("Error en 'Incluye': %s\n", mysql_error(conn));
    } else {
        printf("Error en 'Reserva': %s\n", mysql_error(conn));
    }
}

void agregarAgencia() {
    char nombre[50], direccion[100];

    printf("Nombre de la agencia: ");
    fgets(nombre, sizeof(nombre), stdin);
    nombre[strcspn(nombre, "\n")] = 0;

    printf("Dirección: ");
    fgets(direccion, sizeof(direccion), stdin);
    direccion[strcspn(direccion, "\n")] = 0;

    char query[256];
    snprintf(query, sizeof(query), "INSERT INTO Agencia (NombreA, DireccionA) VALUES ('%s', '%s')", nombre, direccion);
    if (mysql_query(conn, query) == 0)
        printf("Agencia agregada con éxito.\n");
    else
        printf("Error al agregar agencia: %s\n", mysql_error(conn));
}

void agregarGaraje() {
    char nombre[50], direccion[100];

    printf("Nombre del garaje: ");
    fgets(nombre, sizeof(nombre), stdin);
    nombre[strcspn(nombre, "\n")] = 0;

    printf("Dirección: ");
    fgets(direccion, sizeof(direccion), stdin);
    direccion[strcspn(direccion, "\n")] = 0;

    char query[256];
    snprintf(query, sizeof(query), "INSERT INTO Garaje (NombreG, DireccionG) VALUES ('%s', '%s')", nombre, direccion);
    if (mysql_query(conn, query) == 0)
        printf("Garaje agregado con éxito.\n");
    else
        printf("Error al agregar garaje: %s\n", mysql_error(conn));
}

void entregarCoche() {
    int id_reserva;
    printf("ID de la reserva a entregar (se eliminará): ");
    scanf("%d", &id_reserva);
    limpiarBuffer();

    // Primero verificamos que la reserva exista
    char queryVerifica[256];
    snprintf(queryVerifica, sizeof(queryVerifica), "SELECT Estado FROM Reserva WHERE ID_Reserva = %d", id_reserva);
    if (mysql_query(conn, queryVerifica) != 0) {
        printf("Error al verificar reserva: %s\n", mysql_error(conn));
        return;
    }

    res = mysql_store_result(conn);
    if (res == NULL || mysql_num_rows(res) == 0) {
        printf("Reserva no encontrada.\n");
        if (res) mysql_free_result(res);
        return;
    }

    row = mysql_fetch_row(res);
    if (strcmp(row[0], "entregado") == 0) {
        printf("La reserva ya está marcada como entregada.\n");
        mysql_free_result(res);
        return;
    }
    mysql_free_result(res);

    // Eliminamos la reserva (automáticamente se elimina de Incluye gracias al ON DELETE CASCADE)
    char queryDelete[256];
    snprintf(queryDelete, sizeof(queryDelete), "DELETE FROM Reserva WHERE ID_Reserva = %d", id_reserva);

    if (mysql_query(conn, queryDelete) == 0) {
        printf("Reserva entregada y eliminada exitosamente.\n");
    } else {
        printf("Error al eliminar la reserva: %s\n", mysql_error(conn));
    }
}

void consultarReservasPorCliente() {
    int id_cliente;
    printf("ID Cliente: ");
    scanf("%d", &id_cliente);
    limpiarBuffer();

    char query[512];
    snprintf(query,
        sizeof(query),
        "SELECT R.ID_Reserva, R.FechaIn, R.FechaFin, R.Estado, C.Matricula, C.Modelo "
        "FROM Reserva R JOIN Incluye I ON R.ID_Reserva = I.ID_Reserva "
        "JOIN Coche C ON I.Matricula = C.Matricula "
        "WHERE R.ID_Clientes = %d", id_cliente);

    if (mysql_query(conn, query) == 0) {
        res = mysql_store_result(conn);
        while ((row = mysql_fetch_row(res)) != NULL) {
            printf("Reserva: %s | %s a %s | Estado: %s | Coche: %s (%s)\n",
                   row[0], row[1], row[2], row[3], row[4], row[5]);
        }
        mysql_free_result(res);
    } else {
        printf("Error: %s\n", mysql_error(conn));
    }
}

void eliminarCliente() {
    int id;
    printf("ID Cliente a eliminar: ");
    scanf("%d", &id);
    limpiarBuffer();

    char query[100];
    snprintf(query, sizeof(query), "DELETE FROM Clientes WHERE ID_Clientes = %d", id);
    if (mysql_query(conn, query) == 0)
        printf("Cliente eliminado.\n");
    else
        printf("Error: %s\n", mysql_error(conn));
}

// Nueva función para mostrar tablas
void mostrarTabla() {
    int opc;
    printf("\n¿Qué tabla quieres ver?\n");
    printf("1. Clientes\n");
    printf("2. Coches\n");
    printf("3. Agencias\n");
    printf("4. Garajes\n");
    printf("5. Reservas\n");
    printf("Opción: ");
    scanf("%d", &opc);
    limpiarBuffer();

    char query[512];

    switch(opc) {
        case 1:
            strcpy(query, "SELECT ID_Clientes, Nombre, Apellido FROM Clientes");
            break;
        case 2:
            strcpy(query, "SELECT Matricula, Modelo, Marca, Gasolina, Color, Precio, ID_Garaje FROM Coche");
            break;
        case 3:
            strcpy(query, "SELECT ID_Agencia, NombreA, DireccionA FROM Agencia");
            break;
        case 4:
            strcpy(query, "SELECT ID_Garaje, NombreG, DireccionG FROM Garaje");
            break;
        case 5:
            strcpy(query, "SELECT ID_Reserva, FechaIn, FechaFin, PrecioTotal, Estado, ID_Clientes, ID_Agencia FROM Reserva");
            break;
        default:
            printf("Opción inválida.\n");
            return;
    }

    if (mysql_query(conn, query) != 0) {
        printf("Error: %s\n", mysql_error(conn));
        return;
    }

    res = mysql_store_result(conn);
    if (!res) {
        printf("No hay datos.\n");
        return;
    }

    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);

    // Mostrar encabezados
    for (int i = 0; i < num_fields; i++) {
        printf("%-15s", fields[i].name);
    }
    printf("\n");
    printf("===============================================================\n");

    // Mostrar filas
    while ((row = mysql_fetch_row(res)) != NULL) {
        for (int i = 0; i < num_fields; i++) {
            printf("%-15s", row[i] ? row[i] : "NULL");
        }
        printf("\n");
    }

    mysql_free_result(res);
}
void mostrarReservasConJoin() {
    printf("\nReservas con datos completos (JOIN entre Reserva, Clientes, Agencia y Coche):\n");

    const char *query =
        "SELECT R.ID_Reserva, R.FechaIn, R.FechaFin, R.Estado, "
        "C.Nombre, C.Apellido, A.NombreA, Co.Matricula, Co.Modelo "
        "FROM Reserva R "
        "INNER JOIN Clientes C ON R.ID_Clientes = C.ID_Clientes "
        "INNER JOIN Agencia A ON R.ID_Agencia = A.ID_Agencia "
        "INNER JOIN Incluye I ON R.ID_Reserva = I.ID_Reserva "
        "INNER JOIN Coche Co ON I.Matricula = Co.Matricula";

    if (mysql_query(conn, query) != 0) {
        printf("Error al ejecutar el JOIN: %s\n", mysql_error(conn));
        return;
    }

    res = mysql_store_result(conn);
    if (!res) {
        printf("No hay resultados.\n");
        return;
    }

    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);

    // Encabezados
    for (int i = 0; i < num_fields; i++) {
        printf("%-15s", fields[i].name);
    }
    printf("\n");
    printf("==========================================================================\n");

    // Filas
    while ((row = mysql_fetch_row(res)) != NULL) {
        for (int i = 0; i < num_fields; i++) {
            printf("%-15s", row[i] ? row[i] : "NULL");
        }
        printf("\n");
    }

    mysql_free_result(res);
}

void menu() {
    int opc;
    do {
        printf("\n=========== MENÚ ===========\n");
        printf("1. Agregar cliente\n");
        printf("2. Agregar reserva\n");
        printf("3. Entregar coche\n");
        printf("4. Consultar reservas por cliente\n");
        printf("5. Eliminar cliente\n");
        printf("6. Agregar agencia\n");
        printf("7. Agregar garaje\n");
        printf("8. Mostrar tabla\n");
        printf("9. Mostrar reservas con JOIN\n");  // <--- NUEVA OPCIÓN
        printf("10. Salir\n");
        printf("============================\n");
        printf("Opción: ");
        scanf("%d", &opc);
        limpiarBuffer();

        switch (opc) {
            case 1: agregarCliente(); break;
            case 2: hacerReserva(); break;
            case 3: entregarCoche(); break;
            case 4: consultarReservasPorCliente(); break;
            case 5: eliminarCliente(); break;
            case 6: agregarAgencia(); break;
            case 7: agregarGaraje(); break;
            case 8: mostrarTabla(); break;
            case 9: mostrarReservasConJoin(); break; // <--- NUEVA OPCIÓN
            case 10: printf("Saliendo...\n"); break;
            default: printf("Opción inválida.\n");
        }
    } while (opc != 10);
}


int main() {
    conectarBD();
    menu();
    mysql_close(conn);
    return 0;
}
