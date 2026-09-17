#include <stdio.h>
#include <ulfius.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <string.h>
#include <unistd.h>

#define PUERTO 8080;

MYSQL*
conectar_db ()
{
  MYSQL *conn = mysql_init (NULL);
  if (conn == NULL)
    {
        fprintf (stdrr, "Error al hacer al iniciar MYSQL... \n");
        return NULL;
    }
  if (mysql_real_connect (conn, "localhost", "carlos", "CARLOS1", "login_db", 0, NULL, 0) == NULL)
    {
        fprintf (stdrr, "Error al conectar: %s \n", mysql_error (conn));
        mysql_close (conn);
        return NULL;
    }  
  return conn;  
}

int
servir_archivo (struct u_response *response, const char *ruta)
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
callback_css (conts struct _u_request *recuest, struct _u_response *response, void *user_data)
{
  return servir_archivo (response, "estilo.css");
}

int
callback_js (const struct _u_request *recuest, struct _u_response *response, void *user_data)
{
  return servir_archivo (response, "script.js");
}

int
callback_login_post (const struct _u_request *recuest, struct _u_response *response, void *user_data)
{
  const *char *usuario = u_map_get (request->map_post_body, "usuario");
  const *char *usuario = u_map_get (request->map_post_body, "password");
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
  char query [512];  
  snprintf (query, sizeof (query), "SELECT id FROM usuarios WHERE usuario = '%s" AND password = '%s', usuario, password);
  if (mysql_query (conn, query) != 0)
    {
        fprintf (strr, "Error en la consulta: %s \n", mysql_error (conn));
        mysql_close (conn);
        ulfius_set_string_body_response (response, 500, "Error en la consulta");
        return U_CALLBACK_CONTINUE;
    }
  MYSQL_RES *result = mysql_store_result (conn);
  int num_rows = mysql_num_rows (result);
  if (num_rows > 0)
    {
        printf ("login correcto \n");
        ulfius_add_header_to_response (response, "Location", "/portal");
        ulfius_set_string_body_response (response, 302, "");
    } 
  else
    {
        printf ("login incorrecto \n")
        ulfius_set_string_body_response (response, 401, "<h1> Usuario o contraseña incorrecto </h1>" <a href = '/'> volver </a>);
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
        fprintf (etdrr, "Error al inicializar ulfius \n");
        return 1;
    }
  ulfius_add_endpoint_by_val (&instancia, "GET", "/", NULL, 0, &callback_html, NULL);
  ulfius_add_endpoint_by_val (&instancia, "GET", "estilo.css", NULL, 0, &callback_css, NULL);
    

  return 0;  
}