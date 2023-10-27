<template>
    <v-table>
      <thead>
        <tr>
          <th class="text-left">
            Name
          </th>
          <th class="text-left">
            Catégorie
          </th>
        </tr>
      </thead>
      <tbody>
        <tr
          v-for="item in participants"
          :key="item.name"
        >
          <td>{{ item.name }}</td>
          <td>{{ item.category }}</td>
        </tr>
      </tbody>
    </v-table>
  </template>
  <script>
    export default {
      data () {
        return {
          participants: [
            {
              name: 'Clifford Adkins',
              category: 'VTT',
            },
            {
              name: 'Ben Macdonald',
              category: 'Trotinette',
            }
          ],
        }
      },
    }
  </script>