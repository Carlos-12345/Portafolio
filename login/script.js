const usuario = document.getElementById ("usuario");
const clave = document.getElementById ("password");
const ver = document.getElementById ("ver");
const formulario = document.getElementById ("form");
const mensaje = document.getElementById ("mensaje");

ver.addEventListener ("click", function ()
{
  console.log ("ejecutando...");  
  if (clave.type == "password")
    {
        clave.type = "text";
        ver.textContent = "ocultar";
    }
  else
    {
        clave.type = "password";
        ver.textContent = "ver";
    }  
});

formulario.addEventListener ("submit", function (event)
{
  mensaje.textContent = "";
  if (usuario.value.trim () === "")
    {
        event.preventDefault ();
        mensaje.textContent = "Ingresa tu usuario";
        mensaje.style.color = "red";
        return;
    }
  if (clave.value.trim () === "")
    {
        event.preventDefault ();
        mensaje.textContent = "ingresa tu contraseña";
        mensaje.style.color = "red";
        return;
    }  
  mensaje.textContent = "enviando...";
  mensaje.style.color = "#81ff73";  
});