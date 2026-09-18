#include <stdio.h>
#include <ulfius.h>
#include <stdlib.h>
#include <mariadb/mysql.h>
#include <string.h>
#include <unistd.h>
#include <crypt.h>

#define PUERTO 8081

void
hash_password (const char *password, char *output)
{
  char salt[32];
  snprintf (salt, sizeof (salt), "$2b$12$%.22s", "abcdefghijklmnopqrstuv");
  char *hash = crypt (password, salt);
  if (hash == NULL)
    {
        fprintf (stderr, "Error al hashear la contraseña \n");
        strcpy (output, "");
        return;
    }
  strcpy (output, hash);  
}

MYSQL*
conectar_db ()
{
  MYSQL *conn = mysql_init (NULL);
  if (conn == NULL)
    {
        fprintf (stderr, "Error al hacer al iniciar MYSQL... \n");
        return NULL;
    } 
  if (mysql_real_connect (conn, "localhost", "carlos", "CARLOS1", "login_db", 0, NULL, 0) == NULL)
    {
        fprintf (stderr, "Error al conectar: %s \n", mysql_error (conn));
        mysql_close (conn);
        return NULL;
    }  
  return conn;  
}

int
servir_archivo (struct _u_response *response, const char *ruta)
{
  FILE *f = fopen (ruta, "rb");
  if (f == NULL)
    {
        ulfius_set_string_body_response (response, 404, "archivo no encontrado");
        return U_CALLBACK_CONTINUE;
    }
  fseek (f, 0, SEEK_END);
  long fsize = ftell (f);
  fseek (f, 0, SEEK_SET);
  
  char *contenido = malloc (fsize + 1);
  fread (contenido, 1, fsize, f);
  fclose (f);
  contenido [fsize] = '\0';

  ulfius_set_string_body_response (response, 200, contenido);
  free (contenido);
  return U_CALLBACK_CONTINUE;
}

int 
callback_html (const struct _u_request *recuest, struct _u_response *response, void *user_data)
{
  return servir_archivo (response, "index.html");
}

int
callback_css (const struct _u_request *recuest, struct _u_response *response, void *user_data)
{
  return servir_archivo (response, "estilo.css");
}

int
callback_js (const struct _u_request *recuest, struct _u_response *response, void *user_data)
{
  return servir_archivo (response, "script.js");
}

int
callback_login_post (const struct _u_request *request, struct _u_response *response, void *user_data)
{
  const char *usuario = u_map_get (request->map_post_body, "usuario");
  const char *password = u_map_get (request->map_post_body, "password");
  if (usuario == NULL || password == NULL)
    {
        ulfius_set_string_body_response (response, 404, "Datos incompletos");
        return U_CALLBACK_CONTINUE;
    }
  printf ("intento de login: %s / %s \n", usuario, password);
  MYSQL *conn = conectar_db ();
  if (conn == NULL)
    {
        ulfius_set_string_body_response (response, 500, "Error de conexion:");
        return U_CALLBACK_CONTINUE;
    }  
  char query[512];
  snprintf (query, sizeof (query), "SELECT password FROM usuarios WHERE usuario = '%s'", usuario); 
  if (mysql_query (conn, query) != 0)
    {
        fprintf (stderr, "Error en la consulta: %s \n", mysql_error (conn));
        mysql_close (conn);
        ulfius_set_string_body_response (response, 500, "Error en la consulta");
        return U_CALLBACK_CONTINUE;
    }
  MYSQL_RES *result = mysql_store_result (conn);
  MYSQL_ROW row = mysql_fetch_row (result);
  if (row != NULL)
    {
        char *hash_guardado = row[0];
        char hash_ingresado[256];
        hash_password (password, hash_ingresado);
     
        if (strcmp (hash_guardado, hash_ingresado) == 0)
          {
              printf ("Login correcto\n");
              ulfius_add_header_to_response (response, "Location", "/portal");
              ulfius_set_string_body_response (response, 302, "");
          }  
        else
          {
              printf ("Login incorrecto\n");
              ulfius_set_string_body_response (response, 401, "<h1>Usuario o contraseña incorrectos</h1><a href='/'>Volver</a>");
          }
    }      
  else
    {
        printf ("Usuario no encontrado\n");
        ulfius_set_string_body_response (response, 401, "<h1>Usuario o contraseña incorrectos</h1><a href='/'>Volver</a>");
    }  
  mysql_free_result (result);
  mysql_close (conn);
  return U_CALLBACK_CONTINUE;  
}

int
callback_portal (const struct _u_request *request, struct _u_response *response, void *user_data)
{
  ulfius_set_string_body_response (response, 200, "<h1> Bienvenido al portal </h1> <p> login exitoso </p>");
  return U_CALLBACK_CONTINUE;
}

int
main ()
{
  struct _u_instance instancia;

  if (ulfius_init_instance (&instancia, PUERTO, NULL, NULL) != U_OK)
    {
        fprintf (stderr, "Error al inicializar ulfius \n");
        return 1;
    }
  ulfius_add_endpoint_by_val (&instancia, "GET", "/", NULL, 0, &callback_html, NULL);
  ulfius_add_endpoint_by_val (&instancia, "GET", "estilo.css", NULL, 0, &callback_css, NULL);
  ulfius_add_endpoint_by_val (&instancia, "GET", "script.js", NULL, 0, &callback_js, NULL);
  ulfius_add_endpoint_by_val (&instancia, "POST", "/Login", NULL, 0, &callback_login_post, NULL);  
  ulfius_add_endpoint_by_val (&instancia, "GET", "/portal", NULL, 0, &callback_portal, NULL);
  
  if (ulfius_start_framework (&instancia) == U_OK)
    {
        printf ("Servidor corriendo en thhp://localhost: %d \n", PUERTO);
        while (1)
          {
              sleep (1);
          }   
    }
  else 
    {
      fprintf (stderr, "Error al arrancar el framework \n");
    } 

  ulfius_stop_framework (&instancia);
  ulfius_clean_instance (&instancia);  
  return 0;  
}