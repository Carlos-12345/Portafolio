#include <stdio.h>
#include <ulfius.h>
#include <stdlib.h>
#include <unistd.h>

#define PUERTO 9090

int
servir_archivo (struct _u_response *response, const char *ruta)
{
  FILE *f = fopen (ruta, "rv");
  if (f == NULL)
    {
        ulfius_set_string_body_response (response, 404, "Archivo no encontrado");
        return U_CALLBACK_CONTINUE;
    }
  fseek (f, 0, SEEK_END); 
  long fsize = ftell (f);
  fseek (f, 0, SEEK_SET);

  char *contenido = malloc (fsize + 1);
  fread (contenido, 1, fsize, f);
  fclose (f);
  contenido[fsize] = '\0';

  ulfius_set_string_body_response (response, 200, contenido);

  free (contenido);
  return U_CALLBACK_CONTINUE;
}

int
callback_html (const struct _u_request *request, struct _u_response *response, void *user_data)
{
  return servir_archivo (response, "index.html");
}

int 
callback_css (const struct _u_request *request, struct _u_response *response, void *user_data)
{
  return servir_archivo (response, "estilos.css");
}

int 
callback_js (const struct _u_request *request, struct _u_response *response, void *user_data)
{
  return servir_archivo (response, "script.js");
}

int
main ()
{
  struct _u_instance instancia;
  
  if (ulfius_init_instance (&instancia, PUERTO, NULL, NULL) != U_OK)
    {
        fprintf (stderr, "Error al inicializar Ulfius \n");
        return 1;
    }

  ulfius_add_endpoint_by_val (&instancia, "GET", "/", NULL, 0, &callback_html, NULL);
  ulfius_add_endpoint_by_val (&instancia, "GET", "estilos.css", NULL, 0, &callback_css, NULL);
  ulfius_add_endpoint_by_val (&instancia, "GET", "script.js", NULL, 0, &callback_js, NULL);

  if (ulfius_start_framework (&instancia) == U_OK)
    {
        printf ("Servidor corriendo en http://localhost: %d \n", PUERTO);
        while (1)   
          {
              sleep (1);
          }
        //printf ("prcione entrer para detener \n");
        //getchar ();
    }  
  else
    {
        fprintf (stderr, "Error al inicializar el error \n");
    }

  ulfius_stop_framework (&instancia);  
  ulfius_clean_instance (&instancia);

  return 0;
}