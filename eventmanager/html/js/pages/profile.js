import { api } from "../core/api.js";
import { $, $$, show } from "../core/dom.js";
import { header } from "../components/header.js";

header();

const user = await api.auth.getUser();
if (user.logged_in) {
  // Попълва се контейнера с личните данни
  $("#first-name").innerHTML = `${user.first_name ? user.first_name : ""}`;
  $("#last-name").innerHTML = `${user.last_name ? user.last_name : ""}`;
  $("#email").innerHTML = `${user.email}`;
  $("#phone").innerHTML = `${user.phone ? user.phone : ""}`;

  // Контейнер за резервирани събития
  // Ще има такива само ако потребителят е с роля на клиент
  if (user.role == 2) {
    show($("#booked-events"));
  }

  // Съобщение за успех/грешка при редактиране на данни
  const success_message = localStorage.getItem("success_message");
  const error_message = localStorage.getItem("error_message");
  if (success_message) {
    const el = $("#success-message");
    el.textContent = success_message;
    show(el);
    localStorage.removeItem("success_message");
  } else if (error_message) {
    const el = $("#error-message");
    el.textContent = error_message;
    show(el);
    localStorage.removeItem("error_message");
  }

  // Dropdown менюта
  var toggle_list = $$(".change-toggle");
  var toggle_array = [...toggle_list]; // NodeList -> Array
  var form_list = $$(".change-form");
  var form_array = [...form_list];
  var icon_list = $$(".change-icon");
  var icon_array = [...icon_list];
  toggle_array.forEach((toggle, i) => {
    toggle.addEventListener("click", function () {
      const open = form_array[i].style.display === "none";
      form_array[i].style.display = open ? "block" : "none";
      icon_array[i].classList.toggle("fa-chevron-down", !open);
      icon_array[i].classList.toggle("fa-chevron-up", open);
    });
  });
}
// Ако потребителят не е влязъл
else {
  window.location.href = "/login";
}

// Редактиране на имейл
$("#change-email-btn").addEventListener("click", () => {
  const email = $("#new-email").value;
  const password = $("#email-confirm-password").value;
  fetchProfilePatch(
    { email, password },
    "email",
    "Имейлът е сменен успешно.",
  );
});

// Редактиране на телефон
$("#change-phone-btn").addEventListener("click", () => {
  const phone = $("#new-phone").value;
  const password = $("#phone-confirm-password").value;
  fetchProfilePatch(
    { phone, password },
    "phone",
    "Телефонът е сменен успешно.",
  );
});

// Редактиране на парола
$("#change-password-btn").addEventListener("click", () => {
  const current_password = $("#current-password").value;
  const new_password = $("#new-password").value;
  fetchProfilePatch(
    { current_password, new_password },
    "password",
    "Паролата е сменена успешно.",
  );
});

// Modal за изтриване на профил
let modal = $("#delete-modal");

function openModal() {
  if (!modal.classList.contains("is-active")) modal.classList.add("is-active");
}

function closeModal() {
  if (modal.classList.contains("is-active"))
    modal.classList.remove("is-active");
}

$("#delete-btn").addEventListener("click", () => {
  openModal();
});

($$(".modal-background, #modal-close, #modal-cancel") || []).forEach((x) => {
  x.addEventListener("click", () => {
    closeModal();
  });
});

$("#confirm-delete-btn").addEventListener("click", () => {
  deleteAccount();
});

document.addEventListener("keydown", (event) => {
  if (event.key === "Escape") {
    closeModal();
  }
});

function fetchProfilePatch(json, path, message) {
  api.users.updateProfile(json, path);
  if (res.success) {
    localStorage.setItem("success_message", message);
    window.location.reload();
  } else {
    localStorage.setItem("error_message", res.error || "Възникна грешка.");
    window.location.reload();
  }
  window.location.reload();
}

async function deleteAccount() {
  const password = $("#delete-password").value;
  const error = $("#delete-error");

  if (!password) {
    error.textContent = "Моля, въведете паролата си.";
    show(error);
    return;
  }

  const res = await api.users.deleteProfile({ password });
  if (res.success) {
    api.auth.logout();
    window.location.href = "/home";
  } else {
    localStorage.setItem("error_message", res.error || "Възникна грешка.");
    window.location.reload();
  }
}
