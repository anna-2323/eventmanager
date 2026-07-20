import { getUser } from "/js/auth.js";

export function profile() {
  getUser().then((user) => {
    if (user.logged_in) {
      // Попълва се контейнера с личните данни
      document.getElementById("first-name").innerHTML = `${user.first_name ? user.first_name : ""}`;
      document.getElementById("last-name").innerHTML = `${user.last_name ? user.last_name : ""}`;
      document.getElementById("email").innerHTML = `${user.email}`;
      document.getElementById("phone").innerHTML = `${user.phone ? user.phone : ""}`;

      // Контейнер за резервирани събития
      // Ще има такива само ако потребителят е с роля на клиент
      if(user.role == 2) {
        document.getElementById("booked-events").style = "display:block;";
      }

      // Съобщение за успех/грешка при редактиране на данни
      const success_message = localStorage.getItem("success_message");
      const error_message = localStorage.getItem("error_message");
      if (success_message) {
        const el = document.getElementById("success-message");
        el.textContent = success_message;
        el.style.display = "block";
        localStorage.removeItem("success_message");
      } else if (error_message) {
        const el = document.getElementById("error-message");
        el.textContent = error_message;
        el.style.display = "block";
        localStorage.removeItem("error_message");
      }

    // Dropdown менюта
      var toggle_list = document.querySelectorAll(".change-toggle");
      var toggle_array = [...toggle_list]; // NodeList -> Array
      var form_list = document.querySelectorAll(".change-form");
      var form_array = [...form_list];
      var icon_list = document.querySelectorAll(".change-icon");
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
  });

  // Редактиране на имейл
  document
    .getElementById("change-email-btn")
    .addEventListener("click", () => {
      const email = document.getElementById("new-email").value;
      const password = document.getElementById("email-confirm-password").value;
      fetchProfilePatch(
        JSON.stringify({ email, password }),
        "email",
        "Имейлът е сменен успешно."
      );
    });

  // Редактиране на телефон
  document
    .getElementById("change-phone-btn")
    .addEventListener("click", () => {
      const phone = document.getElementById("new-phone").value;
      const password = document.getElementById("phone-confirm-password").value;
      fetchProfilePatch(
        JSON.stringify({ phone, password }),
        "phone",
        "Телефонът е сменен успешно."
      );
    });

  // Редактиране на парола
  document
    .getElementById("change-password-btn")
    .addEventListener("click", () => {
      const current_password = document.getElementById("current-password").value;
      const new_password = document.getElementById("new-password").value;
      fetchProfilePatch(
        JSON.stringify({ current_password, new_password }),
        "password",
        "Паролата е сменена успешно."
      );
    });

}

function fetchProfilePatch(json, path, message) {
  fetch(`/api/profile/${path}`, {
    method: "PATCH",
    headers: { "Content-Type": "application/json" },
    body: json,
  })
  .then(r => r.json())
  .then((res) => {
      if (res.success) {
        localStorage.setItem("success_message", message);
        window.location.reload();
      } else {
        localStorage.setItem("error_message", res.error || "Възникна грешка.");
        window.location.reload();
      }
      window.location.reload();
  })
  .catch(err => console.error('fetch error:', err));
}
