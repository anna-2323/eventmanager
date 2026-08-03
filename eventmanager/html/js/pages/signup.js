import { api } from "../core/api.js";
import { $, show } from "../core/dom.js";
import { header } from "../components/header.js";

header();

$("#submit").addEventListener("click", async function () {
  const first_name = $("#first_name").value.trim();
  const last_name = $("#last_name").value.trim();
  const email = $("#email").value.trim();
  const phone = $("#phone").value.trim();
  const password = $("#password").value.trim();
  var role = 2;
  if ($("#isOrganizer").checked) role = 1;

  if (!first_name || !last_name || !email || !password) {
    error.textContent = "Моля, попълнете всички задължителни полета.";
    show(error);
    return;
  }

  const res = await api.auth.signup(
    { first_name, last_name, email, phone, password, role }
  );
  if (res.success) window.location.href = `/home`;
  else {
    error.textContent = res.error || "Възникна грешка.";
    error.style.display = "block";
  }
});
