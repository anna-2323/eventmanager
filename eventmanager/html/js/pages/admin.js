import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
import { header } from "../components/header.js";
import { createLineChart, createBarChart } from "../components/graph.js";

header();

const user = await api.auth.getUser();
if (!user.logged_in || user.role !== 0) {
  $("#main").innerHTML = `<section class="section">
        <div class="container has-text-centered">
            <h1 class="title has-text-danger">Нямате права за достъп до тази страница</h1>
            <a class="button is-link" href="/home">Начало</a>
        </div>
    </section>`;
} else {
  const monthlyStats = await api.admin.stats.monthly();
  const dailyStats = await api.admin.stats.daily();
  const totals = await api.admin.stats.totals();

  const monthNames = [
    "Яну",
    "Фев",
    "Мар",
    "Апр",
    "Май",
    "Юни",
    "Юли",
    "Авг",
    "Сеп",
    "Окт",
    "Ное",
    "Дек",
  ];

  fillTotalValues();
  fillCharts(monthlyStats);

  function fillTotalValues() {
    $("#total-users").innerHTML = totals.users;
    $("#total-events").innerHTML = totals.events;
    $("#total-venues").innerHTML = totals.venues;
    $("#total-tickets").innerHTML = totals.tickets;
  }

  function fillChart(id, timePeriods, count, name) {
    createBarChart(id, timePeriods, count, name);
  }

  function fillCharts() {
    const months1 = toMonths(monthlyStats.events_growth)
    const event_count = monthlyStats.events_growth.map((x) => x.count);
    fillChart("events-chart", months1, event_count, "Събития");

    const months2 = toMonths(monthlyStats.users_growth)
    const user_count = monthlyStats.users_growth.map((x) => x.count);
    fillChart("users-chart", months2, user_count, "Потребители");

    const months3 = toMonths(monthlyStats.tickets_growth)
    const ticket_count = monthlyStats.tickets_growth.map((x) => x.count);
    fillChart("tickets-chart", months3, ticket_count, "Билети");
  }

  document.addEventListener("change", async (event) => {
    if (event.target.type !== "radio") return;

    const { name, value } = event.target;

    switch (name) {
      case "event-growth":
        if (value === "monthly") {
          // събития помесечно
          const months = toMonths(monthlyStats.events_growth);
          const event_count = monthlyStats.events_growth.map(
            (x) => x.count,
          );
          fillChart("events-chart", months, event_count, "Събития");
        } else {
          // събития подневно
          const days = toDays(dailyStats.events_growth);
          const event_count = dailyStats.events_growth.map(
            (x) => x.count,
          );
          fillChart("events-chart", days, event_count, "Събития");
        }
        break;

      case "user-growth":
        if (value === "monthly") {
          // потребители помесечно
          const months = toMonths(monthlyStats.users_growth);
          const user_count = monthlyStats.users_growth.map(
            (x) => x.count,
          );
          fillChart("users-chart", months, user_count, "Потребители");
        } else {
          // потребители подневно
          const days = toDays(dailyStats.events_growth);
          const user_count = dailyStats.users_growth.map(
            (x) => x.count,
          );
          fillChart("users-chart", days, user_count, "Потребители");
        }
        break;

      case "ticket-growth":
        if (value === "monthly") {
          // билети помесечно
          const months = toMonths(monthlyStats.tickets_growth);
          const ticket_count = monthlyStats.tickets_growth.map(
            (x) => x.count,
          );
          fillChart("tickets-chart", months, ticket_count, "Билети");
        } else {
          // билети подневно
          const days = toDays(dailyStats.tickets_growth);
          const ticket_count = dailyStats.tickets_growth.map(
            (x) => x.count,
          );
          fillChart("ticket-chart", days, ticket_count, "Билети");
        }
        break;
    }
  });

  function toMonths(stats) {
    return stats.map((item) => {
      const month = Number(item.period.split("-")[1]);
      return monthNames[month - 1];
    });
  }

  function toDays(stats) {
    return stats.map((item) => {
      const date = new Date(item.period);
      return date.toLocaleDateString("bg-BG", {
        day: "numeric",
        month: "short",
      });
    });
  }
}
