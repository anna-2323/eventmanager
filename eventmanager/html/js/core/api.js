const BASE_URL = '/api';

async function request(path = {}, options = {}) {
  const res = await fetch(BASE_URL + path, {
    headers: { 'Content-Type': 'application/json', ...options.headers },
    ...options,
    credentials: 'include',
  });
  const data = await res.json();

    if (!res.ok) {
        throw data;
    }

    return data;
}

async function create(path, data, method) {
    const res = await fetch(BASE_URL + path, {
        method: method,
        body: data
    });

    return await res.json();
}

export const api = {
  auth: {
    getUser: () => request('/me'),
    login: (data) => request('/login', { method: 'POST', body: JSON.stringify(data) }),
    logout: () => request('/logout', { method: 'GET' }),
    signup: (data) => request('/signup', { method: 'POST', body: JSON.stringify(data) }),
    forgotPassword: (email) => request('/forgot', { method: 'POST', body: JSON.stringify({ email }) }),
    resetPassword: (data) => request('/reset', { method: 'POST', body: JSON.stringify(data) }),
  },

  cities: {
    list: () => request('/cities')
  },

  categories: {
    list: () => request('/categories')
  },
  
  venues: {
    list: () => request('/venues'),
    get: (id) => request(`/venues/${id}`),
    getEvents: (id) => request(`/venues/${id}/events`),
    getSeatMap: (id) => request(`/venues/seatmap/${id}`)
  },

  events: {
    list: (params) => request('/events' + toQuery(params)),
    get: (id) => request(`/events/${id}`),
    getSeatMap: (id) => request(`/events/seatmap/${id}`)
  },

  tickets: {
    purchase: (id, data) => request(`/purchase/${id}`, { method: 'POST', body: JSON.stringify(data) }),
    confirm: (token) => request(`/confirmation/${token}`),
    edit: (id, data) => request(`/tickets/${id}`, { method: 'PATCH', body: JSON.stringify(data)})
  },

  users: {
    edit: (data) => request('/profile', { method: 'PATCH', body: JSON.stringify(data) }),
    delete: (data) => request('/profile', { method: 'DELETE', body: JSON.stringify(data) }),
    getTickets: (id) => request(`/users/${id}/tickets`),
    getEvents: (id) => request(`/users/${id}/events`)
  },

  stats: {
    monthly: (months) => request(`/stats/monthly${months ? `?months=${months}` : ''}`),
    daily: (months) => request(`/stats/daily${months ? `?months=${months}` : ''}`),
    totals: () => request('/stats/totals'),
    revenue: {
      monthly: (months) => request(`/stats/revenue/monthly${months ? `?months=${months}` : ''}`),
      daily: (months) => request(`/stats/revenue/daily${months ? `?months=${months}` : ''}`),
      byVenues: (months) => request(`/stats/revenue/venues${months ? `?months=${months}` : ''}`)
    },
    // export: {
    //   users: () =>  request('/stats/export/users'),
    //   events: () => request('/stats/export/events'),
    //   tickets: () => request('/stats/export/tickets'),
    //   revenueMonthly: () => request('/stats/export/revenue'),
    //   revenueVenues: () => request('/stats/export/revenue/venues')
    // }
  },

  admin: {
    users: {
      list: () => request('/admin/users'),
      get: (id) => request(`/admin/users/${id}`),
      edit: (id, data) => request(`/admin/users/${id}`, { method: 'PATCH', body: JSON.stringify(data) }),
      delete: (id) => request(`/admin/users/${id}`, { method: 'DELETE' })
    },
    events: {
      list: () => request('/admin/events'),
      get: (id) => request(`/admin/events/${id}`),
      create: (data) => create('/admin/events', data, "POST"),
      edit: (id, data) => request(`/admin/events/${id}`, { method: 'PATCH', body: JSON.stringify(data) }),
      editImage: (id, data) => create(`/admin/events/${id}`, data, "PATCH"),
    },
    venues: {
      list: () => request('/admin/venues'),
      create: (data) => request('/admin/venues', { method: 'POST', body: JSON.stringify(data) }),
      edit: (id, data) => request(`/admin/venues/${id}`, { method: 'PATCH', body: JSON.stringify(data) })
    },
    tickets: {
      list: () => request('/admin/tickets'),
      get: (id) => request(`/admin/tickets/${id}`),
      edit: (id, data) => request(`/admin/tickets/${id}`, {method: 'PATCH', body: JSON.stringify(data)})
    }
  },
};

function toQuery(params) {
  if (!params) return '';

  const filtered = Object.entries(params)
    .filter(([, value]) => value !== null && value !== undefined && value !== '');

  if (filtered.length === 0) {
    return '';
  }

  return '?' + new URLSearchParams(filtered).toString();
}
