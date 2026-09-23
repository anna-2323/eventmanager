import { api } from "../core/api.js";
import { $, show } from "../core/dom.js";
import { header } from "../components/header.js";

header();

$("#submit").addEventListener("click", async function () {
  const email = $("#email").value.trim();
  const password = $("#password").value.trim();

  if (!email || !password) {
    error.textContent = "Моля, попълнете всички полета.";
    error.style.display = "block";
    return;
  }

  try {
    const res = await api.auth.login({ email, password });
    if (res.success) window.location.href = `/home`;
  } catch(err) {
      error.textContent = err.message;
      show(error);
  }

});

$("#password").addEventListener("keydown", function (e) {
  if (e.key === "Enter") $("#submit").click();
});
