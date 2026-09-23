import { api } from "../core/api.js";
import { $, $$, showError, showSuccess, toDate, editItem } from "../core/dom.js";
import { header } from "../components/header.js";
import { input, editSection, activateToggles } from "../components/form.js";
import { userCard } from "../components/adminCards.js";

header();

const { data: user } = await api.auth.getUser();
if (!user.logged_in || user.role !== 0) {
  $("#main").innerHTML = `<section class="section">
        <div class="container has-text-centered">
            <h1 class="title has-text-danger">Нямате права за достъп до тази страница</h1>
            <a class="button is-link" href="/home">Начало</a>
        </div>
    </section>`;
} else {
  const id = window.location.pathname.split("/").pop();
  const { data: editedUser } = await api.admin.users.get(id);
  renderUser();
  renderEvents();
  
  showSuccess();
  showError();

  async function renderUser() {
    $("#user-card").innerHTML = userCard(editedUser);
  }

  document.addEventListener("click", (e) => {
    if(e.target.matches("#events-btn"))
      renderEvents();
    if(e.target.matches("#edit-btn"))
      renderEdit();
    if(e.target.matches("#activate-btn"))
      set_active();
    if(e.target.matches("#delete-btn"))
      renderDelete();

    // Редактиране на потребителски данни
    if(e.target.matches("#change-names-btn")) {
      const first_name = $("#new-first-name").value;
      const last_name = $("#new-last-name").value;
      if(first_name && last_name)
        editItem(api.admin.users.edit, id, { first_name, last_name });
      else {
        localStorage.setItem("error_message", "Моля, въведете нови имена.");
        window.location.reload();
      }
    }
    if(e.target.matches("#change-email-btn")) {
      const email = $("#new-email").value;
      if(email)
        editItem(api.admin.users.edit, id, { email });
      else {
        localStorage.setItem("error_message", "Моля, въведете нов имейл адрес.");
        window.location.reload();
      }
    }
    if(e.target.matches("#change-phone-btn")) {
      const phone = $("#new-phone").value;
      if(phone)
        editItem(api.admin.users.edit, id, { phone });
      else {
        localStorage.setItem("error_message", "Моля, въведете нов телефонен номер.");
        window.location.reload();
      }
    }
    if(e.target.matches("#change-role-btn")) {
      const role = Number($("#role").value);
      editItem(api.admin.users.edit, id, { role });
    }
    if(e.target.matches("#confirm-delete-btn")) {
      deleteUser();
    }
  });
  ($$(".modal-background, #modal-close, #modal-cancel") || []).forEach((x) => {
    x.addEventListener("click", () => {
      closeModal(x.closest(".modal"));
    });
  });

  async function renderEvents() {
    let events;
    if(editedUser.role == 1)
      ({ data: events } = await api.users.getEvents(id));
    if(editedUser.role == 2)
      ({ data: events } = await api.users.getTickets(id));

    $("#changeable").innerHTML = `<h2 id="events-title" class="subtitle">
            ${editedUser.role == 1 ? "Качени събития" : "Купени билети"}
        </h2>`;

    if (events.length > 0) {
      $("#changeable").innerHTML += `
        <table class="table is-fullwidth is-striped is-hoverable" id="events-table">
          <tr> 
            <th>Име</th>
            <th>Време</th>
            <th>Локация</th>
            <th></th>
          </tr>
        </table>`
      $("#events-table").innerHTML += events
        .map(
          (e) =>
            `<tr>
                  <td>${editedUser.role == 1 ? e.title : e.event_name}</td>
                  <td>${toDate(e.begins_at)}</td>
                  <td>${e.venue_name}</td>
                  <td><a href="${user.role == 1 ? 
                      `/admin/events/${e.id}` : `/admin/tickets/${e.id}`
                    }"><button class="button is-small is-link">
                          Преглед
                      </button></a></td>
              </tr>`,
        ).join("");
    } else {
      $("#changeable").innerHTML += "<p>Все още няма събития.</p>";
    }
  }

  function renderEdit() {
    $("#changeable").innerHTML = `
      ${editSection(
        "Промяна на имена",
        "change-names-form",
        `${input("Имена", "new-first-name", "first-name")}
        ${input("", "new-last-name", "last-name")}
        <button class="button is-link" id="change-names-btn">
            Запази
        </button>
        `
      )}

        ${editSection(
          "Промяна на имейл",
          "change-email-form",
          `${input("Нов имейл", "new-email", "email")}
          <button class="button is-link" id="change-email-btn">
              Запази
          </button>
        `
        )}

      ${editSection(
        "Промяна на телефон",
        "change-phone-form",
        `${input("Нов телефон", "new-phone", "tel")}
        <button class="button is-link" id="change-phone-btn">
            Запази
        </button>
        `
      )}

      ${editSection(
        "Промяна на роля",
        "change-role-form",
        `<div class="field">
          <label class="label">Роля</label>
          <div class="control">
            <div class="select">
              <select id="role">
                <option value="0">Администратор</option>
                <option value="1">Организатор</option>
                <option value="2">Потребител</option>
              </select>
            </div>
          </div>
        </div>
        <button class="button is-link" id="change-role-btn">
            Запази
        </button>`
      )}
        `;

    activateToggles();
  }

  function set_active() {
    if(editedUser.active)
        editItem(api.admin.users.edit, id, { active: false });
      else
        editItem(api.admin.users.edit, id, { active: true });
  }

  // Модал
  function openModal(modal) {
  if (!modal.classList.contains("is-active")) modal.classList.add("is-active");
  }

  function closeModal(modal) {
    if (modal.classList.contains("is-active"))
      modal.classList.remove("is-active");
  }

  function renderDelete() {
    openModal($("#delete-modal"));
  }

  async function deleteUser() {
    try {
      const res = await api.admin.users.delete(id);
      if (res.success) {
        localStorage.setItem("success_message", res.message);
        window.location.href = "/admin/users";
      } else {
        localStorage.setItem("error_message", res.message || "Възникна грешка.");
      }
    }
    catch (err) {
      localStorage.setItem("error_message", err.message || "Възникна грешка.");
    }
    finally {
      window.location.reload();
    }
  };
}
