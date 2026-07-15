export function search() {
  document
    .querySelector(".navbar .button")
    .addEventListener("click", function () {
      const query = document
        .querySelector('.navbar input[type="search"]')
        .value.trim();
      if (query)
        window.location.href = `/events?search=${encodeURIComponent(query)}`;
      else
        window.location.href = `/events`;
    });

  // може да се потвърди и с Enter
  document
    .querySelector('.navbar input[type="search"]')
    .addEventListener("keydown", function (e) {
      if (e.key === "Enter") {
        const query = this.value.trim();
        if (query)
          window.location.href = `/events?search=${encodeURIComponent(query)}`;
        else
          window.location.href = `/events`;
      }
    });
}
