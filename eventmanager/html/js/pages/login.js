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

  const res = await api.auth.login({ email, password });
  if (res.success) window.location.href = `/home`;
  else {
    error.textContent = res.error || "Възникна грешка.";
    show(error);
  }
});

$("#password").addEventListener("keydown", function (e) {
  if (e.key === "Enter") $("#submit").click();
});
