import { api } from "../core/api.js";
import { $, show } from "../core/dom.js";
import { header } from "../components/header.js";

header();

$("#submit").addEventListener("click", async function () {
  const email = $("#email").value.trim();

  if (!email) {
    error.textContent = "Моля, попълнете полето.";
    show(error);
    return;
  }

  const res = await api.auth.forgotPassword({ email });
  if (res.success) window.location.href = `/home`;
  else {
    error.textContent = res.error || "Възникна грешка.";
    show(error);
  }
});

$("#email").addEventListener("keydown", function (e) {
  if (e.key === "Enter") $("#submit").click();
});
