<template>
  <v-app>

    <v-app-bar>
      <v-app-bar-nav-icon @click="drawer = !drawer"></v-app-bar-nav-icon>

      <v-app-bar-title>Moniteo Race Manager</v-app-bar-title>


      <v-btn icon="mdi-timer-plus"></v-btn>
    </v-app-bar>


    <v-navigation-drawer v-model="drawer" permanent>
      <v-list nav>
          <v-list-item prepend-icon="mdi-account" title="Participants" value="inbox" :to="{ name: 'Participants'}"></v-list-item>

          <v-divider></v-divider>

          <v-list-subheader inset>Courses</v-list-subheader>

          
          <v-list-item
        v-for="folder in folders"
        :key="folder.title"
        :title="folder.title"
        :subtitle="folder.subtitle"
      >
        <template v-slot:prepend>
          <v-avatar color="grey-lighten-1">
            <v-icon color="white">mdi-bike</v-icon>
          </v-avatar>
        </template>

        <template v-slot:append>
          <v-btn
            color="grey-lighten-1"
            icon="mdi-arrow-right-bold-box"
            variant="text"
            :to=" {name: 'Race', params: { id:'fdsfsqfq' } }"
          ></v-btn>
        </template>
      </v-list-item>




        </v-list>
    </v-navigation-drawer>


    <v-main>
      <v-container>
      <router-view />
      </v-container>
    </v-main>
  </v-app>
</template>

<script lang="ts" setup>
  import { ref } from 'vue'

  const drawer = ref(null)
</script>

<script lang="ts">
  export default {
    data: () => ({ drawer: null,

      folders: [
        {
          id: '1235',
          subtitle: 'Jan 9, 2014',
          title: 'VTT',
        },
        {
          id: 'abcd',
          subtitle: 'Jan 17, 2014',
          title: 'Trotinettes',
        },
        {
          id: 'vdkiri',
          subtitle: 'Jan 28, 2014',
          title: 'Caddie',
        },
      ],

    }),
  }
</script>
