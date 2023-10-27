import { CouchClient } from "./couchdb.ts";

const BOOK_ROUTE = new URLPattern({ pathname: "/books/:id" });

function handler(req: Request): Response {
    const match = BOOK_ROUTE.exec(req.url);
    if (match) {
        const id = match.pathname.groups.id;
        return new Response(`Book ${id}`);
    }
    return new Response("Not found (try /books/1)", {
        status: 404,
    });
}
Deno.serve(handler);



export type User = {
    id: number;
    name: string;
    years: number[];
  };
  async function main() {

    try {
        
    
    // create couch client with endpoint
    const couch = new CouchClient("http://localhost:5984");
    // choose db to use
    const db = couch.database<User>("users");
    // check if specified database exists
    if (!(await couch.databaseExists("users"))) {
      // create new database
      await couch.createDatabase("users");
    }
    // insert new document
    const uesr = {
      id: 100,
      name: "deno",
      years: [2018, 2019],
    };
    const { id, rev } = await db.insert(user);
    // get existing document
    let user = await db.get(id); // {id: 100, name: "deno", years: [2018,2019]}
    // update existing document
    user.years.push(2020);
    await db.put(id, user, { rev });
    // delete existing document
    await db.delete(id);

} catch (error) {
        console.log(error)
}
  }

  main();


