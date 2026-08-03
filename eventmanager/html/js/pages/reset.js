import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
import { header } from "../components/header.js";

header();

const params = new URLSearchParams(window.location.search);
const token = params.get("token");
if (!token) window.location.href = "/home";

$("#submit").addEventListener("click", async function () {
  const new_password = $("#new-password").value.trim();
  const confirm_password = $("#confirm-password").value.trim();

  if (!new_password || !confirm_password) {
    error.textContent = "Моля, попълнете всички полета.";
    show(error);
    return;
  } else if (new_password != confirm_password) {
    error.textContent = "Паролите не съвпадат.";
    show(error);
    return;
  }

  const res = await api.auth.resetPassword(
    { token, new_password }
  );
  if (res.success) window.location.href = `/home`;
  else {
    error.textContent = res.error || "Възникна грешка.";
    show(error);
  }
});

$("#confirm-password").addEventListener("keydown", function (e) {
  if (e.key === "Enter") $("#submit").click();
});
